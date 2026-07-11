/**
 * @file test_audit_helpers.cpp
 * @brief Regressions for the 2026-07-09 audit's divergent-helper findings
 *        (BACKLOG #61).
 *
 * - S_MOVE::to_string(): correct lowercase UCI promotion suffix, "0000" for
 *   the null move; Engine::move_to_uci delegates to it (single formatter).
 * - generate_legal_moves(): preserves the generator's move scores verbatim
 *   (no post-MakeMove re-scoring, no promotion-score reset).
 * - clear_piece_sq64(): king material is never subtracted — symmetric with
 *   add_piece_sq64/rebuild, which never add it.
 * - (Position::perft() and the member generate_all_moves() stub were removed
 *   outright — the compiler now enforces that contract.)
 */

#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/move.hpp"
#include "../src/movegen.hpp"
#include "../src/position.hpp"
#include "../src/search.hpp"
#include "../src/see.hpp"

#include <string>

using namespace Huginn;

namespace {

class AuditHelpersTest : public ::testing::Test {
protected:
    void SetUp() override { Huginn::init(); }  // idempotent (guards itself)
};

// ---- #61: S_MOVE::to_string ------------------------------------------------

TEST_F(AuditHelpersTest, ToStringQuietMove) {
    // e2 = sq64 12, e4 = sq64 28
    S_MOVE m = make_move(12, 28);
    EXPECT_EQ(m.to_string(), "e2e4");
}

TEST_F(AuditHelpersTest, ToStringPromotionIsLowercaseAndCorrectPiece) {
    // e7 = sq64 52, e8 = sq64 60. The old table-indexing bug printed a queen
    // promotion as 'K' (one-based PieceType into a zero-based table) and
    // uppercase where UCI requires lowercase.
    EXPECT_EQ(make_promotion(52, 60, PieceType::Queen).to_string(),  "e7e8q");
    EXPECT_EQ(make_promotion(52, 60, PieceType::Rook).to_string(),   "e7e8r");
    EXPECT_EQ(make_promotion(52, 60, PieceType::Bishop).to_string(), "e7e8b");
    EXPECT_EQ(make_promotion(52, 60, PieceType::Knight).to_string(), "e7e8n");
}

TEST_F(AuditHelpersTest, ToStringNullMove) {
    S_MOVE null_move;
    null_move.move = 0;
    EXPECT_EQ(null_move.to_string(), "0000");
}

TEST_F(AuditHelpersTest, MoveToUciDelegatesToToString) {
    S_MOVE promo = make_promotion(52, 60, PieceType::Queen);
    EXPECT_EQ(Engine::move_to_uci(promo), promo.to_string());
    S_MOVE quiet = make_move(12, 28);
    EXPECT_EQ(Engine::move_to_uci(quiet), quiet.to_string());
}

// ---- #61: generate_legal_moves preserves generator scores -------------------

// Every move that survives the legality filter must appear with the EXACT
// score the pseudo-legal generator assigned (captures were previously
// re-scored after MakeMove — against a board where the source square is
// already empty — and promotion scores were reset by add_quiet_move).
static void expect_scores_preserved(const std::string& fen) {
    Position pos;
    ASSERT_TRUE(pos.set_from_fen(fen));

    S_MOVELIST pseudo;
    generate_all_moves(pos, pseudo);

    S_MOVELIST legal;
    generate_legal_moves(pos, legal);

    ASSERT_GT(legal.count, 0);
    for (int i = 0; i < legal.count; ++i) {
        bool found = false;
        for (int j = 0; j < pseudo.count; ++j) {
            if (pseudo.moves[j].move == legal.moves[i].move) {
                EXPECT_EQ(legal.moves[i].score, pseudo.moves[j].score)
                    << "score clobbered for " << legal.moves[i].to_string()
                    << " in " << fen;
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << legal.moves[i].to_string()
                           << " not in pseudo-legal list for " << fen;
    }
}

TEST_F(AuditHelpersTest, LegalMovesPreserveScoresStartpos) {
    expect_scores_preserved("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

TEST_F(AuditHelpersTest, LegalMovesPreserveScoresKiwipete) {
    // Capture-rich middlegame: the old post-MakeMove re-scoring diverged here.
    expect_scores_preserved(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
}

TEST_F(AuditHelpersTest, LegalMovesPreserveScoresPromotions) {
    // a7 pawn: quiet promotions a8=Q/R/B/N plus capture-promotions on b8.
    expect_scores_preserved("1n6/P7/8/8/8/8/8/k6K w - - 0 1");
}

// ---- #58: SEE first-recapture legality (tracks the built arm) ---------------

// Mirrors the default in src/see.cpp; CMake forwards -DENABLE_SEE_LEGALITY=0/1
// to every translation unit, so this test follows the built arm.
#ifndef ENABLE_SEE_LEGALITY
#define ENABLE_SEE_LEGALITY 0
#endif

// Audit fixture: 4k3/4n3/8/5p2/6Q1/8/8/4R1K1 w — Ne7 is absolutely pinned by
// Re1 to the e8 king, so Qg4xf5 really wins a pawn. Geometric SEE counts the
// illegal Ne7xf5 recapture and scores the capture ~-800; the legality arm
// must score it >= 0 (searched, not hard-pruned by qsearch).
TEST_F(AuditHelpersTest, SeePinnedDefenderRecapture) {
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("4k3/4n3/8/5p2/6Q1/8/8/4R1K1 w - - 0 1"));
    S_MOVE qxf5 = make_capture(30 /*g4*/, 37 /*f5*/, PieceType::Pawn);
#if ENABLE_SEE_LEGALITY
    EXPECT_GE(see(pos, qxf5), 0) << "pinned Ne7 must not count as recapturer";
#else
    EXPECT_LT(see(pos, qxf5), 0) << "geometric arm counts the pinned knight";
#endif
}

// Pin-line exemption: a rook pinned along the e-file CAN still recapture on
// the pin line. Both arms must count it (the exchange on e5 loses the queen).
TEST_F(AuditHelpersTest, SeePinnedDefenderOnPinLineStillCounts) {
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("4k3/8/4r3/4p3/8/8/4Q3/4K3 w - - 0 1"));
    S_MOVE qxe5 = make_capture(12 /*e2*/, 36 /*e5*/, PieceType::Pawn);
    EXPECT_LT(see(pos, qxe5), 0) << "Re6 recaptures along its own pin line";
}

// ---- #57: pinned-position fixture (arm-agnostic sanity) ---------------------

// Knight e2 is pinned by the e8 rook. Its capture Nxc3 scores highest
// (MVV-LVA) so the ORDERED pseudo-legal list starts with illegal entries
// before the first legal king move — the exact shape where the pseudo-legal
// index `i` and the legal-move ordinal diverge (#57). Both arms must search
// it cleanly and return a legal best move; the arms' node counts are compared
// via the run-sheet signatures, and strength via SPRT.
TEST_F(AuditHelpersTest, PinnedPositionSearchReturnsLegalMove) {
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("4r1k1/8/8/8/8/2p5/4N3/4K3 w - - 0 1"));

    Engine engine;
    SearchInfo info;
    info.max_depth = 6;
    S_MOVE best = engine.searchPosition(pos, info);
    ASSERT_NE(best.move, 0);

    S_MOVELIST legal;
    generate_legal_moves(pos, legal);
    bool found = false;
    for (int i = 0; i < legal.count; ++i) {
        if (legal.moves[i].move == best.move) { found = true; break; }
    }
    EXPECT_TRUE(found) << best.to_string() << " is not legal in the fixture";
}

// ---- #61: clear_piece_sq64 king-material symmetry ---------------------------

TEST_F(AuditHelpersTest, ClearPieceMaterialSymmetry) {
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1"));

    // Clearing a real piece subtracts its canonical value.
    const int white_before = pos.material_score[size_t(Color::White)];
    pos.clear_piece_sq64(12);  // e2 pawn
    EXPECT_EQ(pos.material_score[size_t(Color::White)], white_before - 100);

#ifndef DEBUG
    // Kings carry no tracked material: add_piece_sq64/rebuild never add it,
    // so clear must not subtract it either (the old body subtracted
    // value_of(King) = 20000, desynchronizing material on any malformed king
    // removal). Debug builds assert instead of tolerating the call, so this
    // half only runs in Release.
    const int black_before = pos.material_score[size_t(Color::Black)];
    pos.clear_piece_sq64(60);  // e8 black king
    EXPECT_EQ(pos.material_score[size_t(Color::Black)], black_before);
#endif
}

}  // namespace
