// BACKLOG #7 late move pruning re-test (ENABLE_LMP): at a shallow non-PV node,
// once 4 + depth² quiet moves have been searched without a cutoff, the
// remaining quiets are skipped before MakeMove. Behaviour tests are gated on
// the flag (candidate, default OFF); the search-integrity tests run on BOTH
// arms — the pruning must never perturb the baseline arm, and mate lines /
// mate DETECTION must survive the ON arm (LMP skips moves without making
// them, so the legal_count-based mate check needs its one-legal-move guard).

#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/search.hpp"

using namespace Huginn;

namespace {

struct SearchResult {
    S_MOVE best;
    uint64_t nodes;
    uint64_t lmp_prunes;
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
    r.lmp_prunes = info.lmp_prunes;
    return r;
}

}  // namespace

// --- Search integrity (both arms) --------------------------------------------

// Back-rank mate-in-1 (1.Re8#), searched deep: the mate and its score encoding
// must survive pruned move loops (the mate-band + legal_count guards).
TEST(Lmp, BackRankMateSurvivesDeepSearch) {
    Huginn::init();
    const auto r = search_fen("6k1/5ppp/8/8/8/8/8/4R2K w - - 0 1", 8);
    EXPECT_EQ(r.best.get_from(), sq64(File::E, Rank::R1));
    EXPECT_EQ(r.best.get_to(), sq64(File::E, Rank::R8)) << "1.Re8# is forced";
}

// Getting-mated side: from a position where Black is being mated, the search
// must still find the best defense, not prune the defensive quiets away.
// (Same FEN family as the audit's mate fixtures: White mates next move unless
// Black defends.)
TEST(Lmp, DefensiveQuietsSurviveWhenMated) {
    Huginn::init();
    // Black to move, White threatens Re8#; ...Kf8/...quiet defenses must be
    // searched, and best play only delays — search must return a legal move.
    const auto r = search_fen("6k1/5ppp/8/8/8/8/8/4R2K b - - 0 1", 8);
    EXPECT_NE(r.best.move, 0u) << "search must return a legal defensive move";
}

// Single-threaded determinism: two fresh engines at the same depth agree.
TEST(Lmp, FreshEngineSearchIsDeterministic) {
    Huginn::init();
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    const auto a = search_fen(fen, 9);
    const auto b = search_fen(fen, 9);
    EXPECT_EQ(a.best.move, b.best.move);
    EXPECT_EQ(a.nodes, b.nodes);
    EXPECT_EQ(a.lmp_prunes, b.lmp_prunes);
}

// --- Per-arm behaviour --------------------------------------------------------

#if defined(ENABLE_LMP) && ENABLE_LMP

// The whole point: deep searches of quiet-rich positions must actually hit the
// pruning (a shallow non-PV node with > 4 + d² searched quiets). If this stops
// firing, the flag is wired but dead.
TEST(Lmp, PruningFiresInDeepSearch) {
    Huginn::init();
    const auto r = search_fen(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 10);
    EXPECT_GT(r.lmp_prunes, 0u)
        << "no quiet was LMP-pruned anywhere in a depth-10 Kiwipete search";
}

#else  // baseline arm

// The counter must be stone dead on the baseline arm.
TEST(Lmp, BaselineArmNeverPrunes) {
    Huginn::init();
    const auto r = search_fen(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 10);
    EXPECT_EQ(r.lmp_prunes, 0u);
}

#endif
