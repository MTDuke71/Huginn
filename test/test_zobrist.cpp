#include <gtest/gtest.h>
#include "position.hpp"
#include "zobrist.hpp"

TEST(BoardState, StartposCountsAndKeyStable) {
    Zobrist::init_zobrist();
    Position b; b.set_startpos();
    const U64 key1 = Zobrist::compute(b);

    // sanity counts
    EXPECT_EQ(b.piece_counts[size_t(PieceType::Pawn)], 16); // 8 white + 8 black
    EXPECT_EQ(b.piece_counts[size_t(PieceType::King)], 2);  // 1 white + 1 black
    EXPECT_NE(key1, 0u);

    // Rebuild and recompute should match
    b.rebuild_counts();
    const U64 key2 = Zobrist::compute(b);
    EXPECT_EQ(key1, key2);
}
