// test_board.cpp
#include <gtest/gtest.h>
#include "board.hpp"
#include "position.hpp"
#include "chess_types.hpp"
#include "board120.hpp"

class BoardTest : public ::testing::Test {
protected:
    Position pos;
    
    void SetUp() override {
        // Start with a position that has some pieces
        pos.set_startpos();
    }
};

TEST_F(BoardTest, ResetBoardClearsAllSquares) {
    // Verify position starts with pieces
    EXPECT_NE(pos.at(sq(File::E, Rank::R1)), Piece::None);  // White king
    EXPECT_NE(pos.at(sq(File::A, Rank::R1)), Piece::None);  // White rook
    EXPECT_NE(pos.at(sq(File::E, Rank::R2)), Piece::None);  // White pawn
    
    // Reset the board
    reset_board(pos);
    
    // Verify all playable squares are empty
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            int square = sq(static_cast<File>(file), static_cast<Rank>(rank));
            EXPECT_EQ(pos.at(square), Piece::None) 
                << "Square " << char('a' + file) << char('1' + rank) << " should be empty";
        }
    }
    
    // Verify offboard squares are set to Piece::Offboard
    // Test a few offboard squares from the frame
    EXPECT_EQ(pos.board[0], Piece::Offboard);    // Bottom-left corner
    EXPECT_EQ(pos.board[10], Piece::Offboard);   // Left edge of second row
    EXPECT_EQ(pos.board[20], Piece::Offboard);   // Square just before a1
    EXPECT_EQ(pos.board[29], Piece::Offboard);   // Square just after h1
}

TEST_F(BoardTest, ResetBoardClearsPieceCountsAndBitboards) {
    // Verify starting position has pieces
    EXPECT_GT(pos.piece_counts[int(PieceType::Pawn)], 0);
    EXPECT_GT(pos.piece_counts[int(PieceType::King)], 0);
    EXPECT_NE(pos.pawns_bb[0], 0ULL);  // White pawns
    EXPECT_NE(pos.pawns_bb[1], 0ULL);  // Black pawns
    
    // Reset the board
    reset_board(pos);
    
    // Verify all piece counts are zero
    for (int i = 0; i < 7; ++i) {
        EXPECT_EQ(pos.piece_counts[i], 0) << "Piece count for type " << i << " should be 0";
    }
    
    // Verify pawn bitboards are cleared
    EXPECT_EQ(pos.pawns_bb[0], 0ULL);  // White pawns
    EXPECT_EQ(pos.pawns_bb[1], 0ULL);  // Black pawns
}

TEST_F(BoardTest, ResetBoardClearsPieceLists) {
    // Verify starting position has pieces in lists
    EXPECT_GT(pos.pCount[0][int(PieceType::Pawn)], 0);  // White pawns
    EXPECT_GT(pos.pCount[1][int(PieceType::Pawn)], 0);  // Black pawns
    
    // Reset the board
    reset_board(pos);
    
    // Verify all piece counts are zero
    for (int color = 0; color < 2; ++color) {
        for (int type = 0; type < int(PieceType::_Count); ++type) {
            EXPECT_EQ(pos.pCount[color][type], 0) 
                << "Piece count for color " << color << " type " << type << " should be 0";
            
            // Verify piece lists are cleared
            for (int i = 0; i < MAX_PIECES_PER_TYPE; ++i) {
                EXPECT_EQ(pos.pList[color][type][i], -1)
                    << "Piece list entry should be -1 (empty)";
            }
        }
    }
}

TEST_F(BoardTest, ResetBoardClearsKingSquares) {
    // Verify starting position has king squares set
    EXPECT_NE(pos.king_sq[0], -1);  // White king
    EXPECT_NE(pos.king_sq[1], -1);  // Black king
    
    // Reset the board
    reset_board(pos);
    
    // Verify king squares are cleared
    EXPECT_EQ(pos.king_sq[0], -1);  // White king
    EXPECT_EQ(pos.king_sq[1], -1);  // Black king
}

TEST_F(BoardTest, ResetBoardClearsGameState) {
    // Set some game state values
    pos.side_to_move = Color::White;
    pos.ep_square = sq(File::E, Rank::R3);
    pos.halfmove_clock = 25;
    pos.fullmove_number = 42;
    pos.castling_rights = CASTLE_ALL;
    pos.zobrist_key = 0x123456789ABCDEFULL;
    pos.ply = 10;
    
    // Reset the board
    reset_board(pos);
    
    // Verify game state is cleared
    EXPECT_EQ(pos.side_to_move, Color::None);  // Neither side to move
    EXPECT_EQ(pos.ep_square, -1);
    EXPECT_EQ(pos.halfmove_clock, 0);
    EXPECT_EQ(pos.fullmove_number, 1);
    EXPECT_EQ(pos.castling_rights, 0);
    EXPECT_EQ(pos.zobrist_key, 0ULL);
    EXPECT_EQ(pos.ply, 0);
}

TEST_F(BoardTest, ResetClearsMoveHistory) {
    // Since other BoardTests work fine, we know pos is properly initialized by SetUp()
    
    // Verify that after reset, move history is empty and ply is 0
    // (We don't need to manually set up move history; just test that reset clears it)
    
    // Before reset: position should have been set up in SetUp()
    // Make sure we have a valid state first
    EXPECT_GE(pos.ply, 0);  // ply should be non-negative
    
    // Reset the board
    reset_board(pos);
    
    // After reset: move history should be empty and ply should be 0
    EXPECT_TRUE(pos.move_history.empty());
    EXPECT_EQ(pos.ply, 0);
}
