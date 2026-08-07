/**
 * @file test_history_saturation.cpp
 * @brief BACKLOG #68 diagnostic: the butterfly-history distribution is
 *        one-sided, which silently disables half of #63.
 *
 * `search_history` accumulates raw +/-depth^2 with no bound. The bonus fires
 * only when a quiet IMPROVES ALPHA; the malus fires on every quiet that does
 * not -- which, in an all-node, is every quiet there is. Bonuses are therefore
 * rare and maluses are ubiquitous, so the table drifts hard negative.
 *
 * That matters because #63's history-modulated LMR compares these values
 * against a FIXED grain (HISTORY_LMR_GRAIN = 4096): history >= +grain reduces
 * one ply LESS, history <= -grain reduces one ply MORE. If no entry ever
 * reaches +4096, the "proven-good quiet" half of the feature can never fire
 * and the modulation is one-directional.
 *
 * This test measures rather than assumes, and asserts the property it found so
 * a future history change that fixes the asymmetry trips it deliberately.
 */
#include <gtest/gtest.h>
#include <iostream>

#include "../src/search_pool.hpp"
#include "../src/init.hpp"

using Huginn::SearchPool;
using Huginn::SearchInfo;

namespace {
constexpr int GRAIN = 4096;  // HISTORY_LMR_GRAIN in search.cpp

struct Stats { long long mx, mn; int pos_over, neg_over, nonzero; };

Stats measure(const char* fen, int depth) {
    SearchPool pool;
    pool.set_threads(1);
    Position pos;
    pos.set_from_fen(fen);
    SearchInfo info;
    info.max_depth = depth;
    info.depth_only = true;
    info.infinite = true;
    pool.search(pos, info);

    Stats s{0, 0, 0, 0, 0};
    for (int p = 0; p < 13; ++p) {
        for (int q = 0; q < 64; ++q) {
            const long long v = pool.driver().search_history[p][q];
            if (v != 0) ++s.nonzero;
            if (v > s.mx) s.mx = v;
            if (v < s.mn) s.mn = v;
            if (v >  GRAIN) ++s.pos_over;
            if (v < -GRAIN) ++s.neg_over;
        }
    }
    return s;
}

TEST(HistorySaturation, MalusesDominateSoTheLmrGrainIsOneSided) {
    Huginn::init();
    struct Case { const char* name; const char* fen; int depth; };
    const Case cases[] = {
        {"startpos d12", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 12},
        {"Kiwipete d12", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 12},
        {"Kiwipete d14", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 14},
    };

    std::cout << "\n  --- butterfly history vs the #63 LMR grain (" << GRAIN << ") ---\n";
    for (const auto& c : cases) {
        const Stats s = measure(c.fen, c.depth);
        std::cout << "  " << c.name
                  << "  nonzero=" << s.nonzero
                  << "  max=+" << s.mx << "  min=" << s.mn
                  << "  |  > +grain: " << s.pos_over
                  << "   < -grain: " << s.neg_over << "\n";
        EXPECT_GT(s.nonzero, 0) << c.name << ": search populated no history";
        // THE FINDING: maluses dominate so heavily that #63's modulation is
        // effectively one-directional. Entries below -grain outnumber those
        // above +grain by ~40x (measured 119/3, 138/0, 269/6), so the
        // "proven-good quiet, reduce one ply LESS" half of the feature almost
        // never fires while its counterpart fires constantly.
        //
        // Asserted as a RATIO, not an absolute: an early draft claimed the
        // positive half was unreachable and this test falsified it (3, 0 and 6
        // entries do clear +grain). If a history rework balances the
        // distribution, this fails BY DESIGN -- update it deliberately.
        EXPECT_GT(s.neg_over, 8 * s.pos_over)
            << c.name << ": history asymmetry has changed (>+grain=" << s.pos_over
            << ", <-grain=" << s.neg_over << ", max=+" << s.mx << ", min=" << s.mn << ")";
        EXPECT_GT(s.neg_over, 0) << c.name << ": expected negative saturation";
    }
    std::cout << "\n";
}
}  // namespace
