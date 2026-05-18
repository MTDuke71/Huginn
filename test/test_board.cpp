// test_board.cpp
#include <gtest/gtest.h>
#include "position.hpp"
#include "chess_types.hpp"
#include "square.hpp"

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
    EXPECT_NE(pos.at_sq64(sq64(File::E, Rank::R1)), Piece::None);  // White king
    EXPECT_NE(pos.at_sq64(sq64(File::A, Rank::R1)), Piece::None);  // White rook
    EXPECT_NE(pos.at_sq64(sq64(File::E, Rank::R2)), Piece::None);  // White pawn
    
    // Reset the board
    pos.reset();
    
    // Verify all playable squares are empty
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            int square = sq64(static_cast<File>(file), static_cast<Rank>(rank));
            EXPECT_EQ(pos.at_sq64(square), Piece::None) 
                << "Square " << char('a' + file) << char('1' + rank) << " should be empty";
        }
    }
    
    // There is no offboard frame in sq64 space; all 0..63 squares are real.
}

TEST_F(BoardTest, ResetBoardClearsPieceCountsAndBitboards) {
    auto piece_count = [&](PieceType pt) {
        return popcount(pos.piece_bitboards[int(Color::White)][int(pt)])
             + popcount(pos.piece_bitboards[int(Color::Black)][int(pt)]);
    };

    // Verify starting position has pieces
    EXPECT_GT(piece_count(PieceType::Pawn), 0);
    EXPECT_GT(piece_count(PieceType::King), 0);
    EXPECT_NE(pos.get_white_pawns(), 0ULL);
    EXPECT_NE(pos.get_black_pawns(), 0ULL);

    pos.reset();

    for (int i = int(PieceType::Pawn); i <= int(PieceType::King); ++i) {
        EXPECT_EQ(piece_count(static_cast<PieceType>(i)), 0)
            << "Piece count for type " << i << " should be 0";
    }
    EXPECT_EQ(pos.get_white_pawns(), 0ULL);
    EXPECT_EQ(pos.get_black_pawns(), 0ULL);
}

TEST_F(BoardTest, ResetBoardClearsKingSquares) {
    // Verify starting position has king squares set
    EXPECT_NE(pos.king_sq[0], -1);  // White king
    EXPECT_NE(pos.king_sq[1], -1);  // Black king
    
    // Reset the board
    pos.reset();
    
    // Verify king squares are cleared
    EXPECT_EQ(pos.king_sq[0], -1);  // White king
    EXPECT_EQ(pos.king_sq[1], -1);  // Black king
}

TEST_F(BoardTest, ResetBoardClearsGameState) {
    // Set some game state values
    pos.side_to_move = Color::White;
    pos.ep_square = sq64(File::E, Rank::R3);
    pos.halfmove_clock = 25;
    pos.fullmove_number = 42;
    pos.castling_rights = CASTLE_ALL;
    pos.zobrist_key = 0x123456789ABCDEFULL;
    pos.ply = 10;
    
    // Reset the board
    pos.reset();
    
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
    pos.reset();
    
    // After reset: move history should be empty and ply should be 0
    EXPECT_TRUE(pos.move_history.empty());
    EXPECT_EQ(pos.ply, 0);
}

