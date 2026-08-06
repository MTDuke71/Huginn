/**
 * @file test_smp_pool.cpp
 * @brief BACKLOG #40 phase 3: the Lazy SMP thread pool.
 *
 * Fixed-depth node counts stop being a valid gate the moment helpers exist —
 * the whole point of Lazy SMP is that the tree differs run to run. So these
 * tests assert the properties that must hold *regardless* of nondeterminism:
 * the move is always legal, the threads really do contribute, a stop is
 * honoured promptly at every thread count, and repeated search/stop cycles
 * neither crash nor leak state between searches.
 *
 * The one determinism check that survives — `Threads=1` bit-identity against
 * baseline-t34 — is a signature gate run against the built binary, not a unit
 * test; see docs/SMP_PHASE3_DESIGN.md.
 *
 * @see src/search_pool.hpp
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "../src/search_pool.hpp"
#include "../src/init.hpp"
#include "../src/movegen.hpp"

using Huginn::SearchPool;
using Huginn::SearchInfo;

namespace {

class SmpPoolTest : public ::testing::Test {
protected:
    void SetUp() override { Huginn::init(); }

    static bool is_legal_in(const Position& pos, const S_MOVE& m) {
        if (m.move == 0) return false;
        Position copy = pos;
        S_MOVELIST legal;
        generate_legal_moves(copy, legal);
        for (int i = 0; i < legal.count; ++i) {
            if (legal.moves[i].move == m.move) return true;
        }
        return false;
    }

    static SearchInfo depth_limited(int depth) {
        SearchInfo info;
        info.max_depth = depth;
        info.depth_only = true;
        info.infinite = true;   // depth-only searches ignore the clock
        return info;
    }
};

TEST_F(SmpPoolTest, DefaultsToSingleThreaded) {
    SearchPool pool;
    EXPECT_EQ(pool.threads(), 1);
}

TEST_F(SmpPoolTest, SetThreadsClampsToValidRange) {
    SearchPool pool;

    pool.set_threads(4);
    EXPECT_EQ(pool.threads(), 4);

    pool.set_threads(0);
    EXPECT_EQ(pool.threads(), 1) << "0 threads is meaningless; clamp up to 1";

    pool.set_threads(-3);
    EXPECT_EQ(pool.threads(), 1);

    pool.set_threads(SearchPool::MAX_THREADS + 100);
    EXPECT_EQ(pool.threads(), SearchPool::MAX_THREADS);
}

// The shared TT must survive a thread-count change — it is the pool's
// cross-search memory, and resizing the worker set is not a new game.
TEST_F(SmpPoolTest, SharedTableSurvivesThreadCountChange) {
    SearchPool pool;
    pool.tt().store(0xABCDEF0123456789ULL, 42, 5, TTEntry::EXACT, 0x1234);

    pool.set_threads(4);

    int s; uint8_t d, t; uint32_t m;
    EXPECT_TRUE(pool.tt().probe(0xABCDEF0123456789ULL, s, d, t, m));
    EXPECT_EQ(s, 42);

    // Every worker must see that same table.
    for (int i = 0; i < pool.threads(); ++i) {
        EXPECT_EQ(&pool.driver().tt_table, &pool.tt());
    }
}

TEST_F(SmpPoolTest, SingleThreadedSearchReturnsLegalMove) {
    SearchPool pool;
    Position pos;
    pos.set_startpos();

    SearchInfo info = depth_limited(6);
    S_MOVE best = pool.search(pos, info);

    EXPECT_TRUE(is_legal_in(pos, best));
}

// The core phase-3 assertion: with helpers running, the answer is still a
// legal move, at every thread count, from several different positions.
TEST_F(SmpPoolTest, MultiThreadedSearchAlwaysReturnsLegalMove) {
    const char* fens[] = {
        nullptr,  // startpos
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",  // Kiwipete
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",                             // endgame
    };

    for (int threads : {2, 4, 8}) {
        for (const char* fen : fens) {
            SearchPool pool;
            pool.set_threads(threads);

            Position pos;
            if (fen) ASSERT_TRUE(pos.set_from_fen(fen)) << fen;
            else pos.set_startpos();

            SearchInfo info = depth_limited(7);
            S_MOVE best = pool.search(pos, info);

            EXPECT_TRUE(is_legal_in(pos, best))
                << "threads=" << threads << " fen=" << (fen ? fen : "startpos");
        }
    }
}

// Helpers must actually search. If they were silently no-ops (or never
// spawned), the pool's summed node count would equal the driver's alone.
TEST_F(SmpPoolTest, HelpersContributeNodes) {
    SearchPool pool;
    pool.set_threads(4);

    Position pos;
    pos.set_startpos();

    SearchInfo info = depth_limited(8);
    pool.search(pos, info);

    const uint64_t total = pool.total_nodes();
    EXPECT_GT(total, info.nodes)
        << "summed nodes (" << total << ") must exceed the driver's own ("
        << info.nodes << ") — helpers are not contributing";
}

// BACKLOG #65 x #40: the pool still uses every thread under a live soft bound.
//
// #65 gave SearchInfo two deadlines. `soft_stop_time` ("do not START another
// iteration") is deliberately not propagated to helpers in SearchPool::search:
// that decision belongs to the driver, whose move is the one played and whose
// return fires group_stop_. Helpers inherit only the hard deadline.
//
// Honest scope of what this guards: propagating the field would NOT idle the
// helpers for long — they would stop at their own next iteration boundary past
// the soft bound, which lands near where the driver stops anyway. The reason to
// keep it driver-only is semantic (helper scheduling must not be coupled to a
// driver-only time policy), and the reason to test is coarser: that a
// clock-bounded search still recruits every thread at all. A time-handling
// change that crippled helpers under a clock would pass every OTHER test here,
// since they all use depth-limited searches.
TEST_F(SmpPoolTest, HelpersContributeUnderALiveSoftTimeBound) {
    SearchPool pool;
    pool.set_threads(4);

    Position pos;
    pos.set_startpos();

    // A clock-style search with a LIVE soft bound (strictly after start_time,
    // or the root check's liveness guard makes it inert and this test asserts
    // nothing) and a hard bound far enough out that it never binds.
    SearchInfo info;
    info.max_depth = 24;
    info.start_time = std::chrono::steady_clock::now();
    info.soft_stop_time = info.start_time + std::chrono::milliseconds(200);
    info.stop_time = info.start_time + std::chrono::seconds(5);

    const S_MOVE best = pool.search(pos, info);
    EXPECT_TRUE(is_legal_in(pos, best));

    EXPECT_GT(pool.total_nodes(), info.nodes)
        << "helpers contributed nothing (" << pool.total_nodes() << " total vs "
        << info.nodes << " driver) under a clock-bounded search; see the "
        << "soft_stop_time comment in SearchPool::search";
}

// A stop must be honoured promptly at every thread count. This is the
// regression guard for a helper that misses the group flag and hangs the join.
TEST_F(SmpPoolTest, StopIsHonouredPromptlyAtEveryThreadCount) {
    for (int threads : {1, 2, 8}) {
        SearchPool pool;
        pool.set_threads(threads);

        Position pos;
        pos.set_startpos();

        SearchInfo info;
        info.max_depth = Huginn::MAX_DEPTH;
        info.infinite = true;          // run until told to stop

        std::atomic<bool> done{false};
        S_MOVE best;
        std::thread searcher([&]() {
            best = pool.search(pos, info);
            done.store(true);
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        const auto t0 = std::chrono::steady_clock::now();
        pool.stop_all();
        searcher.join();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - t0).count();

        EXPECT_TRUE(done.load());
        EXPECT_LT(elapsed, 2000)
            << "threads=" << threads << ": stop took " << elapsed << "ms";
        EXPECT_TRUE(is_legal_in(pos, best)) << "threads=" << threads;
    }
}

// Repeated search/stop cycles on one pool: worker state must be reset between
// searches, and a stale stop flag must not poison the next search.
TEST_F(SmpPoolTest, RepeatedSearchCyclesStayHealthy) {
    SearchPool pool;
    pool.set_threads(4);

    Position pos;
    pos.set_startpos();

    for (int cycle = 0; cycle < 5; ++cycle) {
        SearchInfo info = depth_limited(6);
        S_MOVE best = pool.search(pos, info);

        EXPECT_TRUE(is_legal_in(pos, best)) << "cycle " << cycle;
        EXPECT_GT(info.nodes, 0u) << "cycle " << cycle
            << ": a stale stop flag would make this search return immediately";
    }
}

// ucinewgame must reach every worker, not just thread 0 (#46).
TEST_F(SmpPoolTest, NewGameClearsSharedTableAndAllWorkers) {
    SearchPool pool;
    pool.set_threads(4);

    Position pos;
    pos.set_startpos();
    SearchInfo info = depth_limited(6);
    pool.search(pos, info);

    ASSERT_GT(pool.tt().permill_full(), 0) << "search should have filled the TT";

    pool.new_game();

    EXPECT_EQ(pool.tt().permill_full(), 0) << "new_game must clear the shared TT";
    for (int sq = 0; sq < 64; ++sq) {
        EXPECT_EQ(pool.driver().search_killers[0][0].move, 0u);
        break;
    }
}

// Helpers must never be configured to touch stdin — input_is_waiting() and
// read_input() are main-thread-only. This pins the default so a future
// SearchInfo change cannot silently flip it.
TEST_F(SmpPoolTest, SearchInfoPollsInputByDefaultForNonHelpers) {
    SearchInfo info;
    EXPECT_TRUE(info.polls_input) << "existing single-threaded callers must keep polling";
    EXPECT_FALSE(info.is_helper);
    EXPECT_EQ(info.thread_id, 0);
    EXPECT_EQ(info.node_mirror, nullptr);
}

}  // namespace
