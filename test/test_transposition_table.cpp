/**
 * @file test_transposition_table.cpp
 * @brief Replacement-policy and #42 date-based-aging tests for the TT
 *        (src/transposition_table.hpp).
 *
 * Uses `TranspositionTable(0)` — resize_mb rounds 0 MB down to a 1-entry
 * table — so ANY two distinct keys collide on slot 0 and the replacement
 * policy is exercised directly. The aging-specific cases are compile-gated
 * on ENABLE_TT_AGING (the experiment/tt-aging arm); the depth-preferred
 * cases hold on both arms (within one search the dates are equal, so the
 * aging clause never fires).
 */

#include <gtest/gtest.h>

#include "../src/transposition_table.hpp"

namespace {

// Arbitrary distinct nonzero keys (0 marks an empty slot, so never use it).
constexpr uint64_t KEY_A = 0x123456789ABCDEF0ULL;
constexpr uint64_t KEY_B = 0x0FEDCBA987654321ULL;

// Probe helper: returns hit flag, fills the out-params only on a hit.
bool probe(const TranspositionTable& tt, uint64_t key,
           int& score, uint8_t& depth, uint8_t& type, uint32_t& move) {
    return tt.probe(key, score, depth, type, move);
}

bool hit(const TranspositionTable& tt, uint64_t key) {
    int s; uint8_t d, t; uint32_t m;
    return probe(tt, key, s, d, t, m);
}

TEST(TranspositionTable, StoreProbeRoundTrip) {
    TranspositionTable tt(1);
    tt.store(KEY_A, 123, 7, TTEntry::LOWER_BOUND, 0xBEEF);

    int s; uint8_t d, t; uint32_t m;
    ASSERT_TRUE(probe(tt, KEY_A, s, d, t, m));
    EXPECT_EQ(s, 123);
    EXPECT_EQ(d, 7);
    EXPECT_EQ(t, TTEntry::LOWER_BOUND);
    EXPECT_EQ(m, 0xBEEFu);
    EXPECT_FALSE(hit(tt, KEY_B));  // different key, no false positives
}

// Within a single search (equal dates) the t22 depth-preferred policy holds
// on BOTH arms: a shallower colliding store must NOT evict a deeper resident.
TEST(TranspositionTable, DepthPreferredWithinOneSearch) {
    TranspositionTable tt(0);  // 1 entry — KEY_A and KEY_B share slot 0
    tt.new_search();
    tt.store(KEY_A, 50, 10, TTEntry::EXACT, 1);
    tt.store(KEY_B, -20, 2, TTEntry::EXACT, 2);  // shallower collision → dropped

    EXPECT_TRUE(hit(tt, KEY_A));
    EXPECT_FALSE(hit(tt, KEY_B));

    tt.store(KEY_B, -20, 10, TTEntry::EXACT, 2);  // equal depth → replaces
    EXPECT_TRUE(hit(tt, KEY_B));
    EXPECT_FALSE(hit(tt, KEY_A));
}

TEST(TranspositionTable, SameKeyAlwaysRefreshes) {
    TranspositionTable tt(0);
    tt.new_search();
    tt.store(KEY_A, 50, 10, TTEntry::EXACT, 1);
    tt.store(KEY_A, 75, 3, TTEntry::UPPER_BOUND, 9);  // same position, shallower → still refreshes

    int s; uint8_t d, t; uint32_t m;
    ASSERT_TRUE(probe(tt, KEY_A, s, d, t, m));
    EXPECT_EQ(s, 75);
    EXPECT_EQ(d, 3);
    EXPECT_EQ(t, TTEntry::UPPER_BOUND);
    EXPECT_EQ(m, 9u);
}

TEST(TranspositionTable, ClearEmptiesTableAndResetsAge) {
    TranspositionTable tt(0);
    tt.new_search();
    tt.store(KEY_A, 50, 10, TTEntry::EXACT, 1);
    tt.clear();  // ucinewgame (#46)
    EXPECT_FALSE(hit(tt, KEY_A));
    EXPECT_EQ(tt.get_current_age(), 0);
}

#if ENABLE_TT_AGING

// (a) The #42 squat fix: a deep entry from search N is evicted in search N+1
// by a depth-2 store for a DIFFERENT key in the same slot — exactly the store
// that the t22 depth-preferred policy drops.
TEST(TranspositionTableAging, StaleDeepEntryEvictedByShallowStoreNextSearch) {
    TranspositionTable tt(0);
    tt.new_search();                               // search N
    tt.store(KEY_A, 50, 10, TTEntry::EXACT, 1);

    tt.new_search();                               // search N+1
    tt.store(KEY_B, -20, 2, TTEntry::EXACT, 2);    // stale resident → evicted

    int s; uint8_t d, t; uint32_t m;
    ASSERT_TRUE(probe(tt, KEY_B, s, d, t, m));
    EXPECT_EQ(d, 2);
    EXPECT_FALSE(hit(tt, KEY_A));
}

// A probe hit re-dates the entry (touch), so a hot entry survives the next
// search's shallow colliding store even though it was written last search.
TEST(TranspositionTableAging, ProbeTouchKeepsHotEntryAlive) {
    TranspositionTable tt(0);
    tt.new_search();
    tt.store(KEY_A, 50, 10, TTEntry::EXACT, 1);

    tt.new_search();
    EXPECT_TRUE(hit(tt, KEY_A));                   // touch: re-dates KEY_A
    tt.store(KEY_B, -20, 2, TTEntry::EXACT, 2);    // no longer stale → depth rule → dropped

    EXPECT_TRUE(hit(tt, KEY_A));
    EXPECT_FALSE(hit(tt, KEY_B));
}

// (b) The age bits packed above the 2-bit bound type must never leak into the
// node_type a caller sees: after many date bumps, probe still returns the
// exact EXACT/LOWER_BOUND/UPPER_BOUND values (0/1/2).
TEST(TranspositionTableAging, ProbeReturnsMaskedNodeType) {
    TranspositionTable tt(1);
    for (int i = 0; i < 37; ++i) tt.new_search();  // nonzero, odd-ish age in the upper bits

    const uint8_t types[] = { TTEntry::EXACT, TTEntry::LOWER_BOUND, TTEntry::UPPER_BOUND };
    for (uint8_t type : types) {
        tt.store(KEY_A, 11, 5, type, 3);
        int s; uint8_t d, t; uint32_t m;
        ASSERT_TRUE(probe(tt, KEY_A, s, d, t, m));
        EXPECT_EQ(t, type);
        EXPECT_LE(t, 2);  // never carries age bits
    }
}

// (c) The 6-bit date wraps at 64 without breaking replacement: 64 bumps later
// a survivor's date reads current again (depth-preferred re-applies); one more
// bump and it is stale again (evictable).
TEST(TranspositionTableAging, AgeWrapsAt64WithoutBreakingReplacement) {
    TranspositionTable tt(0);
    EXPECT_EQ(tt.get_current_age(), 0);
    tt.new_search();
    EXPECT_EQ(tt.get_current_age(), 1);
    tt.store(KEY_A, 50, 10, TTEntry::EXACT, 1);    // stored at date 1

    for (int i = 0; i < 64; ++i) tt.new_search();  // full wrap: date reads 1 again
    EXPECT_EQ(tt.get_current_age(), 1);
    tt.store(KEY_B, -20, 2, TTEntry::EXACT, 2);    // dates equal → depth rule → dropped
    EXPECT_FALSE(hit(tt, KEY_B));                  // (this probe also misses: no touch)

    tt.new_search();                               // date 2 — resident (date 1, untouched) is stale
    tt.store(KEY_B, -20, 2, TTEntry::EXACT, 2);
    EXPECT_TRUE(hit(tt, KEY_B));
    EXPECT_FALSE(hit(tt, KEY_A));
}

#endif  // ENABLE_TT_AGING

}  // namespace
