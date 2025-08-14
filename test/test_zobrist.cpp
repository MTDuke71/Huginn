#include <gtest/gtest.h>
#include "board_state.hpp"
#include "zobrist.hpp"

TEST(BoardState, StartposCountsAndKeyStable) {
    Zobrist::init_zobrist();
    S_BOARD b; set_startpos(b);
    const U64 key1 = Zobrist::compute(b);

    // sanity counts
    EXPECT_EQ(b.pceNum[wP], 8);
    EXPECT_EQ(b.pceNum[bP], 8);
    EXPECT_EQ(b.pceNum[wK], 1);
    EXPECT_EQ(b.pceNum[bK], 1);
    EXPECT_NE(key1, 0u);

    // Rebuild and recompute should match
    rebuild_counts(b);
    const U64 key2 = Zobrist::compute(b);
    EXPECT_EQ(key1, key2);
}