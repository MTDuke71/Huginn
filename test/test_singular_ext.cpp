// BACKLOG #62 singular extensions (ENABLE_SINGULAR_EXT): at a deep node whose
// TT move is far better than every alternative (exclusion search fails low),
// that move is searched one ply deeper. Behaviour tests are gated on the flag
// (candidate, default OFF); the search-integrity tests run on BOTH arms — the
// exclusion plumbing (excluded_move, TT no-cut/no-store, mate handling) must
// never perturb the baseline arm, and mate encoding must survive the ON arm.

#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/search.hpp"

using namespace Huginn;

namespace {

struct SearchResult {
    S_MOVE best;
    uint64_t nodes;
    uint64_t singular_exts;
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
    r.singular_exts = info.singular_exts;
    return r;
}

}  // namespace

// --- Search integrity (both arms) --------------------------------------------

// Back-rank mate-in-1 (1.Re8#), searched DEEP: depth 8 clears
// SINGULAR_MIN_DEPTH so SE-eligible nodes exist throughout the tree, and the
// mate line must survive the machinery — extensions change node counts, never
// mate scores or the forced move.
TEST(SingularExt, BackRankMateSurvivesDeepSearch) {
    Huginn::init();
    const auto r = search_fen("6k1/5ppp/8/8/8/8/8/4R2K w - - 0 1", 8);
    EXPECT_EQ(r.best.get_from(), sq64(File::E, Rank::R1));
    EXPECT_EQ(r.best.get_to(), sq64(File::E, Rank::R8))
        << "1.Re8# is forced";
}

// Single-threaded search is deterministic: two fresh engines at the same depth
// must agree exactly. Guards nondeterminism creep from the exclusion-search
// plumbing (same-ply recursion, TT interactions) on either arm.
TEST(SingularExt, FreshEngineSearchIsDeterministic) {
    Huginn::init();
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    const auto a = search_fen(fen, 9);
    const auto b = search_fen(fen, 9);
    EXPECT_EQ(a.best.move, b.best.move);
    EXPECT_EQ(a.nodes, b.nodes);
    EXPECT_EQ(a.singular_exts, b.singular_exts);
}

// --- Per-arm behaviour --------------------------------------------------------

#if defined(ENABLE_SINGULAR_EXT) && ENABLE_SINGULAR_EXT

// The whole point: on the ON arm, deep searches of ordinary positions must
// actually find singular TT moves (the counter is incremented only after an
// exclusion search fails low). If this stops firing, the flag is wired but dead.
TEST(SingularExt, ExtensionsFireInDeepSearch) {
    Huginn::init();
    const auto r = search_fen(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 10);
    EXPECT_GT(r.singular_exts, 0u)
        << "no exclusion search failed low anywhere in a depth-10 Kiwipete search";
}

#else  // baseline arm

// The counter must be stone dead on the baseline arm: the SE block is compiled
// out, so any nonzero value means the flag gating leaks.
TEST(SingularExt, BaselineArmNeverExtends) {
    Huginn::init();
    const auto r = search_fen(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 10);
    EXPECT_EQ(r.singular_exts, 0u);
}

#endif
