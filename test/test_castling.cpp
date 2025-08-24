#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "board120.hpp"

class CastlingTest : public ::testing::Test {
protected:
    Position pos;
    S_MOVELIST moves;
    
    void SetUp() override {
        pos.reset();
        moves.count = 0;  // Direct clear - faster than function call
    }
    
    // Helper to count castling moves in move list
    int count_castling_moves(const S_MOVELIST& ml) {
        int count = 0;
        for (int i = 0; i < ml.count; ++i) {
            if (ml.moves[i].is_castle()) {
                count++;
            }
        }
        return count;
    }
    
    // Helper to find a specific castling move
    bool has_castling_move(const S_MOVELIST& ml, int from, int to) {
        for (int i = 0; i < ml.count; ++i) {
            if (ml.moves[i].is_castle() && ml.moves[i].get_from() == from && ml.moves[i].get_to() == to) {
                return true;
            }
        }
        return false;
    }
};

TEST_F(CastlingTest, WhiteKingsideCastling) {
    // Set up position where white can castle kingside
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    
    generate_all_moves(pos, moves);
    
    // Should have kingside castling (e1-g1)
    EXPECT_TRUE(has_castling_move(moves, sq(File::E, Rank::R1), sq(File::G, Rank::R1)));
    
    // Count total castling moves (should be 2: kingside and queenside)
    EXPECT_EQ(count_castling_moves(moves), 2);
}

TEST_F(CastlingTest, WhiteQueensideCastling) {
    // Set up position where white can castle queenside
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    
    generate_all_moves(pos, moves);
    
    // Should have queenside castling (e1-c1)
    EXPECT_TRUE(has_castling_move(moves, sq(File::E, Rank::R1), sq(File::C, Rank::R1)));
}

TEST_F(CastlingTest, BlackKingsideCastling) {
    // Set up position where black can castle kingside
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
    
    generate_all_moves(pos, moves);
    
    // Should have kingside castling (e8-g8)
    EXPECT_TRUE(has_castling_move(moves, sq(File::E, Rank::R8), sq(File::G, Rank::R8)));
    
    // Count total castling moves (should be 2: kingside and queenside)
    EXPECT_EQ(count_castling_moves(moves), 2);
}

TEST_F(CastlingTest, BlackQueensideCastling) {
    // Set up position where black can castle queenside
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
    
    generate_all_moves(pos, moves);
    
    // Should have queenside castling (e8-c8)
    EXPECT_TRUE(has_castling_move(moves, sq(File::E, Rank::R8), sq(File::C, Rank::R8)));
}

TEST_F(CastlingTest, NoCastlingRightsWhite) {
    // Set up position where white has no castling rights
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w kq - 0 1");
    
    generate_all_moves(pos, moves);
    
    // Should have no castling moves for white
    EXPECT_FALSE(has_castling_move(moves, sq(File::E, Rank::R1), sq(File::G, Rank::R1)));
    EXPECT_FALSE(has_castling_move(moves, sq(File::E, Rank::R1), sq(File::C, Rank::R1)));
}

TEST_F(CastlingTest, NoCastlingRightsBlack) {
    // Set up position where black has no castling rights
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R b KQ - 0 1");
    
    generate_all_moves(pos, moves);
    
    // Should have no castling moves for black
    EXPECT_FALSE(has_castling_move(moves, sq(File::E, Rank::R8), sq(File::G, Rank::R8)));
    EXPECT_FALSE(has_castling_move(moves, sq(File::E, Rank::R8), sq(File::C, Rank::R8)));
}

TEST_F(CastlingTest, BlockedKingsideCastling) {
    // Set up position where kingside castling is blocked by pieces
    pos.set_from_fen("r3k1nr/8/8/8/8/8/8/R3KB1R w KQkq - 0 1");
    
    generate_all_moves(pos, moves);
    
    // Should not have kingside castling (blocked by bishop on f1)
    EXPECT_FALSE(has_castling_move(moves, sq(File::E, Rank::R1), sq(File::G, Rank::R1)));
    
    // Should still have queenside castling
    EXPECT_TRUE(has_castling_move(moves, sq(File::E, Rank::R1), sq(File::C, Rank::R1)));
}

TEST_F(CastlingTest, BlockedQueensideCastling) {
    // Set up position where queenside castling is blocked by pieces
    pos.set_from_fen("r1b1k2r/8/8/8/8/8/8/RN2K2R w KQkq - 0 1");
    
    generate_all_moves(pos, moves);
    
    // Should not have queenside castling (blocked by knight on b1)
    EXPECT_FALSE(has_castling_move(moves, sq(File::E, Rank::R1), sq(File::C, Rank::R1)));
    
    // Should still have kingside castling
    EXPECT_TRUE(has_castling_move(moves, sq(File::E, Rank::R1), sq(File::G, Rank::R1)));
}

TEST_F(CastlingTest, KingNotOnStartingSquare) {
    // Set up position where king is not on starting square
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R2K3R w KQkq - 0 1");
    
    generate_all_moves(pos, moves);
    
    // Should have no castling moves (king not on e1)
    EXPECT_EQ(count_castling_moves(moves), 0);
}

TEST_F(CastlingTest, RookNotOnStartingSquare) {
    // Set up position where rook is not on starting square
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/4K1RR w KQkq - 0 1");
    
    generate_all_moves(pos, moves);
    
    // Should have no castling moves (rooks not on starting squares)
    EXPECT_EQ(count_castling_moves(moves), 0);
}

TEST_F(CastlingTest, StartingPosition) {
    // Starting position - no castling possible due to pieces in the way
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    generate_all_moves(pos, moves);
    
    // Should have no castling moves (pieces blocking)
    EXPECT_EQ(count_castling_moves(moves), 0);
}
