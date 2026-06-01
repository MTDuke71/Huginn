/**
 * @file test_search_determinism.cpp
 * @brief Regression guard for BACKLOG #30 — the search must be deterministic.
 *
 * `search_history[13][64]` was an uninitialized raw array member, and
 * `clear_search_tables()` *ages* it (/4) rather than zeroing — so a freshly
 * constructed Engine read per-process heap garbage into quiet-move ordering.
 * That made the best move nondeterministic run-to-run in positions decided
 * purely by quiet ordering (e.g. KQ-vs-K: ~20 near-equal winning queen moves,
 * no captures). Fixed by zero-initializing the array (`= {}`). These tests
 * lock that in so the bug class cannot silently return.
 */

#include <gtest/gtest.h>

#include "../src/search.hpp"
#include "../src/evaluation.hpp"
#include "../src/init.hpp"
#include "../src/position.hpp"

#include <memory>
#include <vector>

using namespace Huginn;

namespace {

class SearchDeterminismTest : public ::testing::Test {
protected:
    void SetUp() override { Huginn::init(); }  // idempotent (guards itself)
};

// Direct invariant: a freshly constructed Engine must start with a fully
// zeroed history table — the exact condition that broke (#30). NOTE: this is
// only a best-effort guard. Uninitialized memory is frequently *coincidentally*
// zero (fresh OS pages are zero-filled), so this test can pass even with the
// bug present; that is precisely why the bug was intermittent. The reliable
// guard is FixedDepthSearchIsDeterministic below (multiple engines at distinct
// addresses). With the fix the invariant always holds, so this never
// false-fails and documents the intent.
TEST_F(SearchDeterminismTest, HistoryTableZeroInitialized) {
    Engine engine;
    long long sum = 0;
    for (int piece = 0; piece < 13; ++piece)
        for (int sq = 0; sq < 64; ++sq)
            sum += engine.search_history[piece][sq];
    EXPECT_EQ(sum, 0)
        << "search_history must be zero-initialized (BACKLOG #30); a non-zero "
           "sum means uninitialized garbage leaked into move ordering.";
}

// Behavioral guard: the same position searched to a fixed depth by several
// independently-constructed engines must return the identical best move.
// The engines are all held alive simultaneously so each Engine (and its inline
// search_history) sits at a distinct address — defeating heap reuse that could
// hand back identical garbage on sequential construct/destruct. KQ-vs-K is
// chosen because it is decided entirely by quiet-move ordering, exactly what
// uninitialized history corrupted.
TEST_F(SearchDeterminismTest, FixedDepthSearchIsDeterministic) {
    const std::string kqk = "8/8/8/4k3/8/3QK3/8/8 w - - 0 1";
    const int kEngines = 4;

    std::vector<std::unique_ptr<Engine>> engines;
    for (int i = 0; i < kEngines; ++i)
        engines.push_back(std::make_unique<Engine>());

    std::vector<int> best_moves;
    for (auto& e : engines) {
        Position pos;
        ASSERT_TRUE(pos.set_from_fen(kqk));
        SearchInfo info;
        info.max_depth = 12;
        info.infinite = true;  // bypass time mgmt -> depth-bounded, no timing variance
        best_moves.push_back(e->searchPosition(pos, info).move);  // production search path
    }

    for (int i = 0; i < kEngines; ++i)
        EXPECT_NE(best_moves[i], 0) << "engine " << i << " returned a null move";
    for (int i = 1; i < kEngines; ++i)
        EXPECT_EQ(best_moves[i], best_moves[0])
            << "engine " << i << " disagreed with engine 0 — fixed-depth search "
               "is nondeterministic (BACKLOG #30 regression).";
}

}  // namespace
