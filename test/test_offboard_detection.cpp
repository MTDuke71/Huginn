// test_offboard_detection.cpp - Example of how offboard detection works in move generation
#include <gtest/gtest.h>
#include "board.hpp"
#include "position.hpp"
#include "chess_types.hpp"
#include "board120.hpp"

TEST(OffboardDetection, MoveGenerationCanDetectOffboard) {
    Position pos;
    reset_board(pos);
    
    // Place a white rook on a1
    int a1 = sq(File::A, Rank::R1);
    pos.set(a1, Piece::WhiteRook);
    
    // Simulate move generation going west from a1
    // This should immediately hit an offboard square
    int west_of_a1 = a1 + WEST;  // Should be an offboard square
    
    // This is how move generation can quickly detect offboard
    EXPECT_TRUE(is_offboard(pos.at(west_of_a1)));
    EXPECT_EQ(pos.at(west_of_a1), Piece::Offboard);
    
    // Compare with going east from a1 (should be b1, which is empty)
    int b1 = a1 + EAST;
    EXPECT_FALSE(is_offboard(pos.at(b1)));
    EXPECT_EQ(pos.at(b1), Piece::None);  // Empty square
    
    // Demonstrate the benefit: in move generation, you can do:
    // for (int to = from + direction; !is_offboard(pos.at(to)); to += direction) {
    //     // Process move to 'to'
    //     if (pos.at(to) != Piece::None) break; // Hit a piece, stop sliding
    // }
    // The !is_offboard() check will immediately stop when hitting the border
}

TEST(OffboardDetection, CharRepresentationShowsOffboard) {
    Position pos;
    reset_board(pos);
    
    // Test that offboard squares show as '#'
    int offboard_square = 0;  // Bottom-left corner of 120-array
    EXPECT_EQ(to_char(pos.board[offboard_square]), '#');
    
    // Test that empty playable squares show as '.'
    int e4 = sq(File::E, Rank::R4);
    EXPECT_EQ(to_char(pos.at(e4)), '.');
}
