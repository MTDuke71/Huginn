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
