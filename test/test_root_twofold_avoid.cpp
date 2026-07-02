#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/movegen.hpp"
#include "../src/search.hpp"
#include "../src/uci_utils.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Mirror of the src/search.cpp flag default (BACKLOG #44 follow-up) so this
// test tracks the built arm: CMake forwards -DENABLE_ROOT_TWOFOLD_AVOID=0/1 to
// every target; unset builds get the branch default (test arm = 1).
#ifndef ENABLE_ROOT_TWOFOLD_AVOID
#define ENABLE_ROOT_TWOFOLD_AVOID 1
#endif

namespace {

// BACKLOG #44 follow-up (ENABLE_ROOT_TWOFOLD_AVOID) shared scenario, modelled
// on tools/repro_repetition_44.py shrunk to a deterministic KQK shuffle:
// White (+Q, eval >> WINNING_REPETITION_AVOID_THRESHOLD) has played the
// Qa4-b4 / Kh8-g8 cycle 1.5 times, so the retreat b4a4 recreates the position
// after ply 3 — a SINGLE repetition (2-fold), one move before the 3-fold the
// pre-existing root guard handles. As in the #44 repro, the shuffle move is
// made to look best via a stale WARM-TT winning score for the repeated
// position: the root child at info.ply == 1 passes neither the threefold
// check (count is 2) nor the in-tree Zarkov single-rep rule (gated
// info.ply > 2), so the stale EXACT entry is served straight to the root.
struct RepetitionBaitResult {
    std::string best_uci;
    uint64_t best_key = 0;               // zobrist key after the chosen move
    std::vector<uint64_t> history_keys;  // keys of every prior game position
};

RepetitionBaitResult run_repetition_bait_search() {
    Huginn::init();
    RepetitionBaitResult out;

    Position pos;
    EXPECT_TRUE(pos.set_from_fen("7k/8/8/8/Q7/8/8/6K1 w - - 0 1"));

    // 6 plies of shuffle — enough history for rep counting (needs the
    // post-root-move pos.ply >= 6) and exactly one prior occurrence of the
    // position b4a4 would recreate.
    const std::vector<std::string> shuffle = {"a4b4", "h8g8", "b4a4",
                                              "g8h8", "a4b4", "h8g8"};
    out.history_keys.push_back(pos.zobrist_key);
    for (const auto& uci : shuffle) {
        S_MOVE m = parse_uci_move(uci, pos);
        EXPECT_NE(m.move, 0) << "history move not legal: " << uci;
        EXPECT_EQ(pos.MakeMove(m), 1) << "history move rejected: " << uci;
        out.history_keys.push_back(pos.zobrist_key);
    }
    EXPECT_EQ(pos.ply, 6);

    Huginn::Engine engine;

    // Scenario validity: b4a4 is legal and is a single repetition (its key
    // occurred exactly once in the game history), NOT a threefold. Seed the
    // warm-TT bait for that position: EXACT, black (the side to move there)
    // catastrophically lost, depth 64 so it survives every probe — the root
    // then sees b4a4 as +2500, above every honest move (~+1290).
    {
        Position probe = pos;
        S_MOVE rep_move = parse_uci_move("b4a4", probe);
        EXPECT_NE(rep_move.move, 0);
        EXPECT_EQ(probe.MakeMove(rep_move), 1);
        int prior = 0;
        for (uint64_t k : out.history_keys)
            if (k == probe.zobrist_key) ++prior;
        EXPECT_EQ(prior, 1) << "b4a4 should recreate exactly one prior key";

        S_MOVELIST replies;
        generate_legal_moves(probe, replies);
        EXPECT_GT(replies.count, 0);
        engine.tt_table.store(probe.zobrist_key, -2500, 64, TTEntry::EXACT,
                              static_cast<uint32_t>(replies.moves[0].move));
    }

    Huginn::SearchInfo info;
    info.max_depth = 6;
    info.infinite = true;  // no clock — depth-limited only

    std::ostringstream captured;  // swallow the UCI info lines
    auto* old_buf = std::cout.rdbuf(captured.rdbuf());
    const S_MOVE best = engine.searchPosition(pos, info);
    std::cout.rdbuf(old_buf);

    EXPECT_NE(best.move, 0);
    out.best_uci = Huginn::Engine::move_to_uci(best);

    Position after = pos;
    EXPECT_EQ(after.MakeMove(best), 1);
    out.best_key = after.zobrist_key;
    return out;
}

}  // namespace

// Test arm: a WINNING engine must not pick a root move that recreates a
// position key already in the game history, even when a stale TT score makes
// the shuffle look best — the clamp steers the root elsewhere.
TEST(RootTwofoldAvoid, WinningRootRoutesAroundSingleRepetition) {
#if !ENABLE_ROOT_TWOFOLD_AVOID
    GTEST_SKIP() << "baseline arm: root guard fires only on threefolds";
#else
    const RepetitionBaitResult r = run_repetition_bait_search();

    EXPECT_NE(r.best_uci, "b4a4")
        << "winning root re-entered a single repetition";

    // The general contract: the chosen move must not recreate ANY position
    // key already seen in the game (b4a4 is the only candidate here, but
    // assert the invariant, not the move).
    for (uint64_t k : r.history_keys) {
        EXPECT_NE(r.best_key, k) << "chosen root move " << r.best_uci
                                 << " recreates a prior game position";
    }
#endif
}

// Baseline arm (-DENABLE_ROOT_TWOFOLD_AVOID=0): documents the hole this flag
// plugs — the identical scenario takes the stale-TT bait and shuffles into
// the single repetition. If this ever fails on the baseline arm, the hole got
// fixed elsewhere and the flag should be re-evaluated.
TEST(RootTwofoldAvoid, BaselineArmTakesStaleTTRepetitionBait) {
#if ENABLE_ROOT_TWOFOLD_AVOID
    GTEST_SKIP() << "test arm: covered by WinningRootRoutesAroundSingleRepetition";
#else
    const RepetitionBaitResult r = run_repetition_bait_search();
    EXPECT_EQ(r.best_uci, "b4a4")
        << "baseline arm no longer takes the repetition bait";
#endif
}
