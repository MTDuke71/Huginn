/**
 * @file test_smp_shared_resources.cpp
 * @brief BACKLOG #40 phase 2: the Engine sharing constructor.
 *
 * Lazy SMP builds N Engines around ONE transposition table and opening book;
 * everything else an Engine owns (history, killers, counter-moves, PV table,
 * node counter, stop flag) must stay private to its worker. These tests pin
 * both halves of that contract, plus the property the whole phase rests on:
 * a default-constructed Engine is still fully self-contained, so every
 * existing single-threaded call site and test is unaffected.
 *
 * @see src/search.hpp (Engine's two constructors)
 */

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "../src/search.hpp"
#include "../src/init.hpp"

using Huginn::Engine;
using Huginn::PolyglotBook;

namespace {

constexpr uint64_t KEY_A = 0x123456789ABCDEF0ULL;
constexpr uint64_t KEY_B = 0x0FEDCBA987654321ULL;

bool hit(const TranspositionTable& tt, uint64_t key) {
    int s; uint8_t d, t; uint32_t m;
    return tt.probe(key, s, d, t, m);
}

// Two engines built around one table see each other's stores — this is the
// entire point of Lazy SMP (workers cooperate through the shared TT).
TEST(SmpSharedResources, WorkersShareOneTranspositionTable) {
    TranspositionTable shared_tt(1);
    PolyglotBook shared_book;

    Engine w0(shared_tt, shared_book, nullptr);
    Engine w1(shared_tt, shared_book, nullptr);

    w0.tt_table.store(KEY_A, 123, 7, TTEntry::EXACT, 0xBEEF);

    int score; uint8_t depth, type; uint32_t move;
    ASSERT_TRUE(w1.tt_table.probe(KEY_A, score, depth, type, move))
        << "worker 1 must see worker 0's store — they share one table";
    EXPECT_EQ(score, 123);
    EXPECT_EQ(depth, 7);
    EXPECT_EQ(type, TTEntry::EXACT);
    EXPECT_EQ(move, 0xBEEFu);

    // ...and the shared table is the very object the caller owns, not a copy.
    EXPECT_TRUE(hit(shared_tt, KEY_A));
    EXPECT_EQ(&w0.tt_table, &shared_tt);
    EXPECT_EQ(&w1.tt_table, &shared_tt);
    EXPECT_EQ(&w0.opening_book, &shared_book);
}

// The flip side: per-thread ordering state must NOT be shared, or workers
// would corrupt each other's move ordering (and history/killers are exactly
// what Lazy SMP relies on diverging).
TEST(SmpSharedResources, WorkersDoNotShareOrderingTables) {
    TranspositionTable shared_tt(1);
    PolyglotBook shared_book;

    Engine w0(shared_tt, shared_book, nullptr);
    Engine w1(shared_tt, shared_book, nullptr);

    w0.search_history[1][20] = 4242;
    EXPECT_EQ(w1.search_history[1][20], 0)
        << "history must be private to a worker";

    w0.counter_moves[10][30].move = 0x1234;
    EXPECT_EQ(w1.counter_moves[10][30].move, 0u)
        << "counter-moves must be private to a worker";

    w0.search_killers[3][0].move = 0x5678;
    EXPECT_EQ(w1.search_killers[3][0].move, 0u)
        << "killers must be private to a worker";

    // The stop token's local channel is per-Engine too; the SMP stop path
    // fans out to each worker rather than relying on one shared atomic.
    w0.stop();
    EXPECT_TRUE(w0.stop_token.stop_requested());
    EXPECT_FALSE(w1.stop_token.stop_requested());
}

// A default-constructed Engine still owns its own TT and book. This is what
// keeps all 18 existing `Engine engine;` test sites (and the UCI layer)
// working unchanged across phase 2.
TEST(SmpSharedResources, DefaultConstructedEngineIsSelfContained) {
    Engine a;
    Engine b;

    a.tt_table.store(KEY_B, 55, 4, TTEntry::EXACT, 0x99);

    EXPECT_TRUE(hit(a.tt_table, KEY_B));
    EXPECT_FALSE(hit(b.tt_table, KEY_B))
        << "independently constructed engines must not share a table";
    EXPECT_NE(&a.tt_table, &b.tt_table);
    EXPECT_NE(&a.opening_book, &b.opening_book);

    // Default table is the advertised 64 MB worth of entries.
    EXPECT_EQ(a.tt_table.get_size(), (64ull * 1024 * 1024) / sizeof(TTEntry));
}

// Resizing through one worker resizes the shared table for all of them — the
// UCI `Hash` option has to reach every worker, not just the one it was
// applied to. (Resize also clears, hence the store-then-resize ordering.)
TEST(SmpSharedResources, HashResizeThroughOneWorkerAffectsAll) {
    TranspositionTable shared_tt(1);
    PolyglotBook shared_book;

    Engine w0(shared_tt, shared_book, nullptr);
    Engine w1(shared_tt, shared_book, nullptr);

    w0.tt_table.store(KEY_A, 1, 1, TTEntry::EXACT, 1);
    ASSERT_TRUE(hit(w1.tt_table, KEY_A));

    w0.tt_table.resize_mb(2);

    EXPECT_EQ(w1.tt_table.get_size(), (2ull * 1024 * 1024) / sizeof(TTEntry));
    EXPECT_FALSE(hit(w1.tt_table, KEY_A)) << "resize discards entries for everyone";
}

// Each worker searches into the shared table with its own private state, and
// a real search through the sharing constructor produces a legal move. Guards
// against the sharing ctor skipping init (init_mvv_lva / clear_search_tables)
// that the default one performs.
TEST(SmpSharedResources, SharedWorkerRunsARealSearch) {
    Huginn::init();

    TranspositionTable shared_tt(8);
    PolyglotBook shared_book;
    Engine worker(shared_tt, shared_book, nullptr);

    Position pos;
    pos.set_startpos();

    Huginn::SearchInfo info;
    info.max_depth = 6;
    info.depth_only = true;
    info.infinite = true;   // depth-only: ignore the clock

    S_MOVE best = worker.searchPosition(pos, info);

    ASSERT_NE(best.move, 0u) << "a shared-resource worker must return a move";

    S_MOVELIST legal;
    generate_legal_moves(pos, legal);
    bool found = false;
    for (int i = 0; i < legal.count; ++i) {
        if (legal.moves[i].move == best.move) { found = true; break; }
    }
    EXPECT_TRUE(found) << "returned move must be legal at the root";

    // The search populated the *shared* table, which is what a helper thread
    // contributes back to the pool.
    EXPECT_GT(shared_tt.permill_full(), 0);
}

}  // namespace
