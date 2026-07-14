// BACKLOG #63 history-modulated LMR (ENABLE_HISTORY_LMR): the mover's
// butterfly-history score adjusts the static LMR table reduction by ±1 ply.
// Behaviour tests are gated on the flag (candidate, default OFF); the
// search-integrity tests run on BOTH arms — the modulation must never perturb
// the baseline arm, and mate lines must survive the ON arm.

#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/search.hpp"

using namespace Huginn;

namespace {

struct SearchResult {
    S_MOVE best;
    uint64_t nodes;
    uint64_t history_lmr_adjusts;
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
    r.history_lmr_adjusts = info.history_lmr_adjusts;
    return r;
}

}  // namespace

// --- Search integrity (both arms) --------------------------------------------

// Back-rank mate-in-1 (1.Re8#), searched deep: reduction changes must never
// cost a forced mate (checks and captures are LMR-exempt, but the surrounding
// tree reshapes — the mate must survive regardless).
TEST(HistoryLmr, BackRankMateSurvivesDeepSearch) {
    Huginn::init();
    const auto r = search_fen("6k1/5ppp/8/8/8/8/8/4R2K w - - 0 1", 8);
    EXPECT_EQ(r.best.get_from(), sq64(File::E, Rank::R1));
    EXPECT_EQ(r.best.get_to(), sq64(File::E, Rank::R8)) << "1.Re8# is forced";
}

// Single-threaded search is deterministic: two fresh engines at the same depth
// must agree exactly (guards nondeterminism creep from the history read).
TEST(HistoryLmr, FreshEngineSearchIsDeterministic) {
    Huginn::init();
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    const auto a = search_fen(fen, 9);
    const auto b = search_fen(fen, 9);
    EXPECT_EQ(a.best.move, b.best.move);
    EXPECT_EQ(a.nodes, b.nodes);
    EXPECT_EQ(a.history_lmr_adjusts, b.history_lmr_adjusts);
}

// --- Per-arm behaviour --------------------------------------------------------

#if defined(ENABLE_HISTORY_LMR) && ENABLE_HISTORY_LMR

// The whole point: deep searches must actually hit the modulation (a quiet
// move whose |history| crossed the grain at an LMR-eligible node). If this
// stops firing, the flag is wired but dead.
TEST(HistoryLmr, ModulationFiresInDeepSearch) {
    Huginn::init();
    const auto r = search_fen(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 12);
    EXPECT_GT(r.history_lmr_adjusts, 0u)
        << "no LMR reduction was history-adjusted in a depth-12 Kiwipete search";
}

#else  // baseline arm

// The counter must be stone dead on the baseline arm.
TEST(HistoryLmr, BaselineArmNeverAdjusts) {
    Huginn::init();
    const auto r = search_fen(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 12);
    EXPECT_EQ(r.history_lmr_adjusts, 0u);
}

#endif
