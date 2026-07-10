/**
 * @file test_audit_criticals.cpp
 * @brief Regressions for the 2026-07-09 independent code audit's critical
 *        findings (BACKLOG #52-#55).
 *
 * - #55: S_MOVELIST appends are bounds-checked (fail closed, no overflow).
 * - #54: set_from_fen is transactional and strictly bounded; the UCI
 *        `position` command is all-or-nothing and structurally validated.
 * - #52: quiescence is check-aware (evasions, horizon mate, quiet
 *        promotions) — candidate arm, ENABLE_QSEARCH_CHECK_EVASIONS.
 * - #53: TT probe/store is bypassed within reach of the rule-50 boundary —
 *        candidate arm, ENABLE_RULE50_TT_GUARD.
 */

#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/movegen.hpp"
#include "../src/position.hpp"
#include "../src/search.hpp"
#include "../src/uci.hpp"
#include "../src/uci_utils.hpp"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

// These mirror the defaults in src/search.cpp. When a candidate arm is built,
// CMake forwards the same -DENABLE_X=1 to every translation unit, so the
// tests track the built arm (same pattern as test_root_twofold_avoid.cpp).
#ifndef ENABLE_QSEARCH_CHECK_EVASIONS
#define ENABLE_QSEARCH_CHECK_EVASIONS 0
#endif
#ifndef ENABLE_RULE50_TT_GUARD
#define ENABLE_RULE50_TT_GUARD 0
#endif

using namespace Huginn;

namespace {

class AuditCriticalsTest : public ::testing::Test {
protected:
    void SetUp() override { Huginn::init(); }  // idempotent (guards itself)

    static std::vector<std::string> tokens(const std::string& command) {
        std::vector<std::string> out;
        std::istringstream iss(command);
        std::string token;
        while (iss >> token) out.push_back(token);
        return out;
    }

    /// A syntactically broken FEN must be rejected AND leave the position
    /// exactly as it was (BACKLOG #54 transactionality).
    static void expect_fen_rejected_and_unchanged(Position& pos, const std::string& bad_fen) {
        const std::string before_fen = pos.to_fen();
        const uint64_t before_key = pos.zobrist_key;
        const size_t before_history = pos.move_history.size();
        EXPECT_FALSE(pos.set_from_fen(bad_fen)) << "accepted malformed FEN: " << bad_fen;
        EXPECT_EQ(pos.to_fen(), before_fen) << "position mutated by rejected FEN: " << bad_fen;
        EXPECT_EQ(pos.zobrist_key, before_key) << "zobrist mutated by rejected FEN: " << bad_fen;
        EXPECT_EQ(pos.move_history.size(), before_history) << "history mutated by rejected FEN: " << bad_fen;
    }
};

// ---------------------------------------------------------------------------
// #55: bounded move-list writes
// ---------------------------------------------------------------------------

// Composed position with 279 pseudo-legal moves (audit fixture): the old
// unchecked appends wrote entries 257..279 past the array. The checked
// primitive must truncate at capacity and leave the list valid.
TEST_F(AuditCriticalsTest, MoveListOverflowFailsClosed) {
#ifdef DEBUG
    GTEST_SKIP() << "debug arm: S_MOVELIST::full() asserts on overflow by design";
#else
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("QQQQQQQk/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1"));

    S_MOVELIST list;
    generate_all_moves(pos, list);

    EXPECT_EQ(list.count, MAX_POSITION_MOVES)
        << "expected the 279-move board to fill the list to capacity exactly";
    for (int i = 0; i < list.count; ++i) {
        EXPECT_GE(list.moves[i].get_from(), 0);
        EXPECT_LT(list.moves[i].get_from(), 64);
        EXPECT_GE(list.moves[i].get_to(), 0);
        EXPECT_LT(list.moves[i].get_to(), 64);
    }
#endif
}

// The same board must never reach the engine through UCI: piece-count
// validation is the deliberate second boundary in front of the move list.
TEST_F(AuditCriticalsTest, MegaPieceBoardRejectedAtUciBoundary) {
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("QQQQQQQk/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1"));
    EXPECT_FALSE(validate_uci_position(pos));
}

// ---------------------------------------------------------------------------
// #54: strict, transactional FEN parsing
// ---------------------------------------------------------------------------

TEST_F(AuditCriticalsTest, MalformedFenCorpusRejectedWithoutMutation) {
    Position pos;
    pos.set_startpos();  // known-good state the rejects must preserve

    const char* corpus[] = {
        "",                                          // empty
        "k7/8/8/8/8/8/8/K7 w - -",                   // 4 tokens
        "k7/8/8/8/8/8/8/K7 w - - 0",                 // 5 tokens
        "k7/8/8/8/8/8/8/K7 w - - 0 1 extra",         // 7 tokens
        "k7/8/8/8/8/8/8/K7 w - - -1 1",              // negative halfmove
        "k7/8/8/8/8/8/8/K7 w - - 12junk 1",          // suffix junk (stoi took 12)
        "k7/8/8/8/8/8/8/K7 w - - 0 0",               // fullmove < 1
        "k7/8/8/8/8/8/8/K7 w - - 0 123456",          // fullmove out of range
        "k7/8/8/8/8/8/9/K7 w - - 0 1",               // digit 9
        "k7/8/8/8/8/8/08/K7 w - - 0 1",              // digit 0
        "k7/8/8/8/8/8/45/K7 w - - 0 1",              // digits summing past 8
        "kk7/8/8/8/8/8/8/K7 w - - 0 1",              // 9 files in a rank
        "k7/8/8/8/8/8/8/K7/8 w - - 0 1",             // 9 ranks
        "k7/8/8/8/8/8/K7 w - - 0 1",                 // 7 ranks
        "k6/8/8/8/8/8/8/K7 w - - 0 1",               // short rank (7 files)
        "k7/8/8/8/8/8/8/J7 w - - 0 1",               // invalid piece letter
        "k7/8/8/8/8/8/8/K7 x - - 0 1",               // bad side to move
        "k7/8/8/8/8/8/8/K7 w KX - 0 1",              // bad castling char
        "k7/8/8/8/8/8/8/K7 w KK - 0 1",              // duplicate castling right
        "k7/8/8/8/8/8/8/K7 w - e4 0 1",              // EP on impossible rank
        "k7/8/8/8/8/8/8/K7 w - i6 0 1",              // EP file out of range
        "k7/8/8/8/8/8/8/K7 w - e3 0 1",              // rank-3 EP but white to move
        "k7/8/8/8/8/8/8/K7 b - e6 0 1",              // rank-6 EP but black to move
    };
    for (const char* bad : corpus) {
        expect_fen_rejected_and_unchanged(pos, bad);
    }
}

TEST_F(AuditCriticalsTest, WellFormedFenStillAccepted) {
    Position pos;
    EXPECT_TRUE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
    EXPECT_TRUE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"));
    EXPECT_TRUE(pos.set_from_fen("k7/8/8/8/8/8/7Q/7K w - - 98 1"));       // high clock is legal
    EXPECT_TRUE(pos.set_from_fen("8/8/8/8/8/8/8/8 w - - 0 1"));           // partial positions stay
    EXPECT_TRUE(pos.set_from_fen("4k3/8/8/8/8/8/8/8 b - - 0 1"));         //  available to unit tests
}

// set_from_fen stays permissive about piece structure (unit tests build
// partial positions through it); validate_uci_position is where the engine's
// root-position assumptions are enforced.
TEST_F(AuditCriticalsTest, StructuralValidatorGatesTheUciBoundary) {
    Position pos;

    ASSERT_TRUE(pos.set_from_fen("8/8/8/8/8/8/8/8 w - - 0 1"));            // no kings
    EXPECT_FALSE(validate_uci_position(pos));

    ASSERT_TRUE(pos.set_from_fen("Kk6/8/8/8/8/8/8/7K w - - 0 1"));         // two white kings
    EXPECT_FALSE(validate_uci_position(pos));

    ASSERT_TRUE(pos.set_from_fen("k7/8/8/8/8/8/8/KP6 w - - 0 1"));         // pawn on rank 1
    EXPECT_FALSE(validate_uci_position(pos));

    ASSERT_TRUE(pos.set_from_fen("k7/8/8/8/8/8/8/K6R w K - 0 1"));         // right K but king not on e1
    EXPECT_FALSE(validate_uci_position(pos));

    ASSERT_TRUE(pos.set_from_fen("k7/8/8/8/8/8/5q2/4K3 b - - 0 1"));       // idle side (White) in check
    EXPECT_FALSE(validate_uci_position(pos));

    ASSERT_TRUE(pos.set_from_fen("k7/8/8/8/8/8/8/K7 b - e3 0 1"));         // EP square without the pushed pawn
    EXPECT_FALSE(validate_uci_position(pos));

    // And the good cases pass.
    ASSERT_TRUE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
    EXPECT_TRUE(validate_uci_position(pos));
    ASSERT_TRUE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"));
    EXPECT_TRUE(validate_uci_position(pos));
    ASSERT_TRUE(pos.set_from_fen("k7/8/8/8/8/8/7Q/7K w - - 98 1"));
    EXPECT_TRUE(validate_uci_position(pos));
}

// The UCI `position` command is all-or-nothing: a bad FEN, an illegal
// position, or ANY bad move in the list rejects the whole command and leaves
// the previous root untouched (the old code kept a silently-desynced prefix).
TEST_F(AuditCriticalsTest, UciPositionCommandIsTransactional) {
    UCIInterface uci;

    uci.handle_position(tokens("position startpos moves e2e4"));
    const std::string good_fen = uci.current_position().to_fen();
    EXPECT_EQ(good_fen, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");

    // Illegal third move: e4e5 is blocked after e7e5. Nothing may change.
    uci.handle_position(tokens("position startpos moves e2e4 e7e5 e4e5"));
    EXPECT_EQ(uci.current_position().to_fen(), good_fen)
        << "a rejected move list must not commit its valid prefix";

    // Nonsense move text.
    uci.handle_position(tokens("position startpos moves e2e4 zz99"));
    EXPECT_EQ(uci.current_position().to_fen(), good_fen);

    // Malformed FEN.
    uci.handle_position(tokens("position fen k7/8/8/8/8/8/8/K7 w - - -1 1"));
    EXPECT_EQ(uci.current_position().to_fen(), good_fen);

    // Syntactically valid but structurally illegal FEN (two white kings).
    uci.handle_position(tokens("position fen Kk6/8/8/8/8/8/8/7K w - - 0 1"));
    EXPECT_EQ(uci.current_position().to_fen(), good_fen);

    // Mega-piece board: rejected by the piece-count gate (#55 second boundary).
    uci.handle_position(tokens("position fen QQQQQQQk/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1"));
    EXPECT_EQ(uci.current_position().to_fen(), good_fen);

    // A subsequent good command still works.
    uci.handle_position(tokens("position fen k7/8/8/8/8/8/7Q/7K w - - 98 1 moves h2b8"));
    EXPECT_EQ(uci.current_position().to_fen(), "kQ6/8/8/8/8/8/8/7K b - - 99 1");
}

// ---------------------------------------------------------------------------
// #52: check-aware quiescence (candidate arm)
// ---------------------------------------------------------------------------

// Audit repro: from 7k/8/5KQ1/8/8/8/8/8 w, `go depth 1` chose g6g5 (~cp 1277)
// because the horizon qsearch stand-patted through Black's checkmate. With
// check-aware qsearch, depth 1 must find g6g7#.
TEST_F(AuditCriticalsTest, QsearchFindsMateInOneAtDepthOne) {
#if !ENABLE_QSEARCH_CHECK_EVASIONS
    GTEST_SKIP() << "baseline arm: qsearch is not check-aware (BACKLOG #52)";
#else
    Engine engine;
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("7k/8/5KQ1/8/8/8/8/8 w - - 0 1"));

    SearchInfo info{};
    info.max_depth = 1;
    info.infinite = true;
    const S_MOVE best = engine.searchPosition(pos, info);

    EXPECT_EQ(best.get_from(), sq64(File::G, Rank::R6));
    EXPECT_EQ(best.get_to(), sq64(File::G, Rank::R7))
        << "depth 1 must see g6g7# through the quiescence horizon";
#endif
}

// Direct qsearch contract: a checkmated side to move returns -MATE + ply.
TEST_F(AuditCriticalsTest, QsearchDetectsHorizonCheckmate) {
#if !ENABLE_QSEARCH_CHECK_EVASIONS
    GTEST_SKIP() << "baseline arm: qsearch is not check-aware (BACKLOG #52)";
#else
    Engine engine;
    Position pos;
    // Position after g6g7# from the audit repro: Black is checkmated.
    ASSERT_TRUE(pos.set_from_fen("7k/6Q1/5K2/8/8/8/8/8 b - - 0 1"));

    SearchInfo info{};
    info.infinite = true;
    const int score = engine.quiescence(pos, -INFINITE, INFINITE, info, 0);
    EXPECT_EQ(score, -MATE) << "checkmate at qsearch ply 0 must encode as -MATE + 0";
#endif
}

// In check with a quiet-only escape: qsearch must search the evasion (the
// capture-only frontier had no legal move here and stand-pat was meaningless).
TEST_F(AuditCriticalsTest, QsearchSearchesQuietEvasions) {
#if !ENABLE_QSEARCH_CHECK_EVASIONS
    GTEST_SKIP() << "baseline arm: qsearch is not check-aware (BACKLOG #52)";
#else
    Engine engine;
    Position pos;
    // White king h1 checked by the a1 queen; the only evasions are the quiet
    // king steps g2/h2. Not mate — the score must stay far from the mate band
    // (Black is a queen up, so it will be very negative, and that is fine).
    ASSERT_TRUE(pos.set_from_fen("k7/8/8/8/8/8/8/q6K w - - 0 1"));

    SearchInfo info{};
    info.infinite = true;
    const int score = engine.quiescence(pos, -INFINITE, INFINITE, info, 0);
    EXPECT_GT(score, -MATE + 100) << "a legal quiet evasion exists; this is not mate";
    EXPECT_LT(score, 0) << "White is a queen down; the evasion score must reflect it";
#endif
}

// Quiet promotions belong to the tactical frontier: a pawn on the 7th must
// be worth (nearly) a queen at the horizon, not a pawn.
TEST_F(AuditCriticalsTest, QsearchSeesQuietPromotions) {
#if !ENABLE_QSEARCH_CHECK_EVASIONS
    GTEST_SKIP() << "baseline arm: quiet promotions not in qsearch (BACKLOG #52)";
#else
    Engine engine;
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("k7/5P2/8/8/8/8/8/K7 w - - 0 1"));

    SearchInfo info{};
    info.infinite = true;
    const int score = engine.quiescence(pos, -INFINITE, INFINITE, info, 0);
    EXPECT_GT(score, 500) << "f7-f8=Q is quiet; the old capture-only frontier missed it";
#endif
}

// info.ply now advances through qsearch, so seldepth must exceed the nominal
// depth on any tactical position.
TEST_F(AuditCriticalsTest, QsearchExtendsSeldepth) {
#if !ENABLE_QSEARCH_CHECK_EVASIONS
    GTEST_SKIP() << "baseline arm: qsearch does not advance info.ply (BACKLOG #52)";
#else
    Engine engine;
    Position pos;  // Kiwipete: dense tactics guarantee qsearch work at the horizon
    ASSERT_TRUE(pos.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"));

    SearchInfo info{};
    info.max_depth = 4;
    info.infinite = true;
    (void)engine.searchPosition(pos, info);
    EXPECT_GT(info.seldepth, 4) << "seldepth must include quiescence plies";
#endif
}

// ---------------------------------------------------------------------------
// #53: rule-50-aware TT eligibility (candidate arm)
// ---------------------------------------------------------------------------

// Audit repro: warming an engine on the same placement at clock 98 poisoned
// the TT with draw-propagated scores that a later clock-0 search of the same
// placement then trusted (~cp 25 / h2c7 instead of the fresh ~cp 1211 /
// h2d6). With the guard, the near-boundary search stores nothing, so the
// warm engine must match a fresh-engine oracle exactly (single-threaded
// determinism makes this a strict equality).
TEST_F(AuditCriticalsTest, Rule50WarmSearchDoesNotPoisonColdSearch) {
#if !ENABLE_RULE50_TT_GUARD
    GTEST_SKIP() << "baseline arm: rule-50 TT guard off (BACKLOG #53)";
#else
    const std::string clock98 = "k7/8/8/8/8/8/7Q/7K w - - 98 1";
    const std::string clock0 = "k7/8/8/8/8/8/7Q/7K w - - 0 1";
    const int kDepth = 2;

    // Fresh-engine oracle at clock 0.
    Engine fresh;
    Position pos_oracle;
    ASSERT_TRUE(pos_oracle.set_from_fen(clock0));
    SearchInfo info_oracle{};
    info_oracle.max_depth = kDepth;
    info_oracle.infinite = true;
    const int oracle_move = fresh.searchPosition(pos_oracle, info_oracle).move;
    ASSERT_NE(oracle_move, 0);

    // Warm an engine at the rule-50 boundary, then search clock 0 with it.
    Engine warm;
    Position pos98;
    ASSERT_TRUE(pos98.set_from_fen(clock98));
    SearchInfo info98{};
    info98.max_depth = kDepth;
    info98.infinite = true;
    (void)warm.searchPosition(pos98, info98);

    Position pos0;
    ASSERT_TRUE(pos0.set_from_fen(clock0));
    SearchInfo info0{};
    info0.max_depth = kDepth;
    info0.infinite = true;
    const int warm_move = warm.searchPosition(pos0, info0).move;

    EXPECT_EQ(warm_move, oracle_move)
        << "clock-98 warm-up leaked a rule-50-contaminated TT entry into the clock-0 search";

    // Reverse order: a clock-0 warm-up must not let the clock-98 search take
    // TT cutoffs its rule-50 context invalidates. (The guard refuses cutoffs
    // when halfmove_clock + depth >= 100, so the search completes on live
    // recomputation; this exercises the probe side of the guard.)
    Position pos98_again;
    ASSERT_TRUE(pos98_again.set_from_fen(clock98));
    SearchInfo info98b{};
    info98b.max_depth = kDepth;
    info98b.infinite = true;
    const int warm98_move = warm.searchPosition(pos98_again, info98b).move;
    EXPECT_NE(warm98_move, 0);
#endif
}

}  // namespace
