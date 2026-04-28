#include <gtest/gtest.h>
#include "position.hpp"
#include "zobrist.hpp"

TEST(BoardState, StartposCountsAndKeyStable) {
    Zobrist::init_zobrist();
    Position b; b.set_startpos();
    const U64 key1 = Zobrist::compute(b);

    // sanity counts
    auto piece_count = [&](PieceType pt) {
        return popcount(b.piece_bitboards[int(Color::White)][int(pt)])
             + popcount(b.piece_bitboards[int(Color::Black)][int(pt)]);
    };
    EXPECT_EQ(piece_count(PieceType::Pawn), 16);
    EXPECT_EQ(piece_count(PieceType::King), 2);
    EXPECT_NE(key1, 0u);

    // Rebuild and recompute should match
    b.rebuild_counts();
    const U64 key2 = Zobrist::compute(b);
    EXPECT_EQ(key1, key2);
}
