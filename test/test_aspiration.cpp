// BACKLOG #17-r2 aspiration windows (ENABLE_ASPIRATION): from depth 6 on, the
// root searches a narrow window centred on the previous depth's score and
// widens geometrically on a fail. Behaviour tests are gated on the flag
// (candidate, default OFF); the search-integrity tests run on BOTH arms — the
// retry loop (window resets, fail-low/high widening, PV reset per pass) must
// never change WHAT the search finds, only how many nodes it costs, and mate
// scores must survive the windowing on the ON arm.

#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/search.hpp"

using namespace Huginn;

namespace {

struct SearchResult {
    S_MOVE best;
    uint64_t nodes;
    uint64_t aspiration_researches;
};

SearchResult search_fen(const std::string& fen, int depth) {
    Engine engine;
    Position pos;
    EXPECT_TRUE(pos.set_from_fen(fen));
    SearchInfo info{};
    info.max_depth = depth;
    info.infinite = true;
    SearchResult r;
    r.best = engine.searchPosition(pos, info);
    r.nodes = info.nodes;
    r.aspiration_researches = info.aspiration_researches;
    return r;
}

}  // namespace

// --- Search integrity (both arms) --------------------------------------------

// Back-rank mate-in-1 (1.Re8#), searched DEEP: depth 8 clears
// ASPIRATION_MIN_DEPTH so windowed iterations run with a mate-range centre —
// the mate-bound full-window fallback must engage and the forced move survive.
TEST(Aspiration, BackRankMateSurvivesDeepSearch) {
    Huginn::init();
    const auto r = search_fen("6k1/5ppp/8/8/8/8/8/4R2K w - - 0 1", 8);
    EXPECT_EQ(r.best.get_from(), sq64(File::E, Rank::R1));
    EXPECT_EQ(r.best.get_to(), sq64(File::E, Rank::R8))
        << "1.Re8# is forced";
}

// Single-threaded search is deterministic: two fresh engines at the same depth
// must agree exactly. Guards nondeterminism creep from the retry loop (window
// state, per-pass PV resets, TT interactions across re-searches) on either arm.
TEST(Aspiration, FreshEngineSearchIsDeterministic) {
    Huginn::init();
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    const auto a = search_fen(fen, 9);
    const auto b = search_fen(fen, 9);
    EXPECT_EQ(a.best.move, b.best.move);
    EXPECT_EQ(a.nodes, b.nodes);
    EXPECT_EQ(a.aspiration_researches, b.aspiration_researches);
}

// --- Per-arm behaviour --------------------------------------------------------

#if defined(ENABLE_ASPIRATION) && ENABLE_ASPIRATION

// The whole point: on the ON arm, positions whose score moves more than the
// +/-50cp window between iterations must trigger a widening re-search (the
// counter increments only on a fail-low/high). Kiwipete's score drops by ~57cp
// crossing ASPIRATION_MIN_DEPTH (measured d5 +4 -> d6 -53 on t31), so a
// depth-8 search must fail at least once. If this stops firing, the flag is
// wired but dead.
TEST(Aspiration, WindowFailuresFireInDeepSearch) {
    Huginn::init();
    const auto r = search_fen(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 8);
    EXPECT_GT(r.aspiration_researches, 0u)
        << "no window fail anywhere in a depth-8 Kiwipete search";
}

#else  // baseline arm

// The counter must be stone dead on the baseline arm: the aspiration block is
// compiled out, so any nonzero value means the flag gating leaks.
TEST(Aspiration, BaselineArmNeverResearches) {
    Huginn::init();
    const auto r = search_fen(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 8);
    EXPECT_EQ(r.aspiration_researches, 0u);
}

#endif
