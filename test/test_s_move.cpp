#include <gtest/gtest.h>
#include "move.hpp"
#include "square.hpp"
#include "chess_types.hpp"

class S_MoveTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(S_MoveTest, BasicMoveEncoding) {
    // Test basic move encoding and decoding
    S_MOVE move(sq64(File::E, Rank::R2), sq64(File::E, Rank::R4));
    
    EXPECT_EQ(move.get_from(), sq64(File::E, Rank::R2));
    EXPECT_EQ(move.get_to(), sq64(File::E, Rank::R4));
    EXPECT_EQ(move.get_captured(), PieceType::None);
    EXPECT_FALSE(move.is_en_passant());
    EXPECT_FALSE(move.is_pawn_start());
    EXPECT_EQ(move.get_promoted(), PieceType::None);
    EXPECT_FALSE(move.is_castle());
    EXPECT_FALSE(move.is_capture());
    EXPECT_FALSE(move.is_promotion());
    EXPECT_TRUE(move.is_quiet());
}

TEST_F(S_MoveTest, CaptureMove) {
    // Test capture move
    S_MOVE move(sq64(File::E, Rank::R4), sq64(File::D, Rank::R5), PieceType::Pawn);
    
    EXPECT_EQ(move.get_from(), sq64(File::E, Rank::R4));
    EXPECT_EQ(move.get_to(), sq64(File::D, Rank::R5));
    EXPECT_EQ(move.get_captured(), PieceType::Pawn);
    EXPECT_TRUE(move.is_capture());
    EXPECT_FALSE(move.is_quiet());
}

TEST_F(S_MoveTest, EnPassantMove) {
    // Test en passant move
    S_MOVE move(sq64(File::E, Rank::R5), sq64(File::D, Rank::R6), PieceType::Pawn, true);
    
    EXPECT_TRUE(move.is_en_passant());
    EXPECT_TRUE(move.is_capture());
    EXPECT_EQ(move.get_captured(), PieceType::Pawn);
    EXPECT_FALSE(move.is_quiet());
}

TEST_F(S_MoveTest, PawnStartMove) {
    // Test pawn double push
    S_MOVE move(sq64(File::E, Rank::R2), sq64(File::E, Rank::R4), PieceType::None, false, true);
    
    EXPECT_TRUE(move.is_pawn_start());
    EXPECT_FALSE(move.is_capture());
    EXPECT_TRUE(move.is_quiet());
}

TEST_F(S_MoveTest, PromotionMove) {
    // Test promotion move
    S_MOVE move(sq64(File::A, Rank::R7), sq64(File::A, Rank::R8), PieceType::None, false, false, PieceType::Queen);
    
    EXPECT_EQ(move.get_promoted(), PieceType::Queen);
    EXPECT_TRUE(move.is_promotion());
    EXPECT_FALSE(move.is_capture());
    EXPECT_FALSE(move.is_quiet());
}

TEST_F(S_MoveTest, PromotionWithCapture) {
    // Test promotion with capture
    S_MOVE move(sq64(File::A, Rank::R7), sq64(File::B, Rank::R8), PieceType::Rook, false, false, PieceType::Queen);
    
    EXPECT_EQ(move.get_promoted(), PieceType::Queen);
    EXPECT_EQ(move.get_captured(), PieceType::Rook);
    EXPECT_TRUE(move.is_promotion());
    EXPECT_TRUE(move.is_capture());
    EXPECT_FALSE(move.is_quiet());
}

TEST_F(S_MoveTest, CastleMove) {
    // Test castle move
    S_MOVE move(sq64(File::E, Rank::R1), sq64(File::G, Rank::R1), PieceType::None, false, false, PieceType::None, true);
    
    EXPECT_TRUE(move.is_castle());
    EXPECT_FALSE(move.is_capture());
    EXPECT_FALSE(move.is_quiet());
}

TEST_F(S_MoveTest, ConvenienceFunctions) {
    // Test convenience creation functions
    auto quiet_move = make_move(sq64(File::D, Rank::R2), sq64(File::D, Rank::R4));
    EXPECT_TRUE(quiet_move.is_quiet());
    
    auto capture = make_capture(sq64(File::E, Rank::R4), sq64(File::D, Rank::R5), PieceType::Pawn);
    EXPECT_TRUE(capture.is_capture());
    EXPECT_EQ(capture.get_captured(), PieceType::Pawn);
    
    auto en_passant = make_en_passant(sq64(File::E, Rank::R5), sq64(File::D, Rank::R6));
    EXPECT_TRUE(en_passant.is_en_passant());
    
    auto pawn_start = make_pawn_start(sq64(File::E, Rank::R2), sq64(File::E, Rank::R4));
    EXPECT_TRUE(pawn_start.is_pawn_start());
    
    auto promotion = make_promotion(sq64(File::A, Rank::R7), sq64(File::A, Rank::R8), PieceType::Queen);
    EXPECT_TRUE(promotion.is_promotion());
    EXPECT_EQ(promotion.get_promoted(), PieceType::Queen);
    
    auto castle = make_castle(sq64(File::E, Rank::R1), sq64(File::G, Rank::R1));
    EXPECT_TRUE(castle.is_castle());
}

TEST_F(S_MoveTest, MoveScoring) {
    // Test move scoring and comparison
    S_MOVE move1 = make_move(sq64(File::E, Rank::R2), sq64(File::E, Rank::R4));
    S_MOVE move2 = make_capture(sq64(File::E, Rank::R4), sq64(File::D, Rank::R5), PieceType::Pawn);
    
    move1.score = 100;
    move2.score = 200;
    
    EXPECT_TRUE(move2 > move1);
    EXPECT_TRUE(move1 < move2);
    EXPECT_FALSE(move1 == move2);
}

TEST_F(S_MoveTest, BitMaskValidation) {
    // Test that all bit fields work correctly with extreme values
    
    // Test maximum square values (63 for 64-square board)
    S_MOVE move(63, 63, PieceType(15), true, true, PieceType(15), true);

    EXPECT_EQ(move.get_from(), 63);
    EXPECT_EQ(move.get_to(), 63);
    EXPECT_EQ(int(move.get_captured()), 15);
    EXPECT_TRUE(move.is_en_passant());
    EXPECT_TRUE(move.is_pawn_start());
    EXPECT_EQ(int(move.get_promoted()), 15);
    EXPECT_TRUE(move.is_castle());
}

TEST_F(S_MoveTest, ConstructorEncoding) {
    // Encode a pawn double-push e2-e4 via the S_MOVE constructor. (This used to
    // exercise the static S_MOVE::encode_move helper, which was deprecated and
    // removed — the constructor performs the identical inline bit encoding.)
    S_MOVE move(
        sq64(File::E, Rank::R2),
        sq64(File::E, Rank::R4),
        PieceType::None,
        false,
        true,  // pawn start
        PieceType::None,
        false
    );

    EXPECT_EQ(move.get_from(), sq64(File::E, Rank::R2));
    EXPECT_EQ(move.get_to(), sq64(File::E, Rank::R4));
    EXPECT_TRUE(move.is_pawn_start());
    EXPECT_FALSE(move.is_en_passant());
    EXPECT_FALSE(move.is_castle());
}

