#include <gtest/gtest.h>
#include "position.hpp"
#include "init.hpp"
#include "board120.hpp"
#include "chess_types.hpp"

class MaterialTrackingTest : public ::testing::Test {
protected:
    void SetUp() override {
        Huginn::init();
        pos.set_startpos();
    }
    
    Position pos;
};

TEST_F(MaterialTrackingTest, StartingPositionMaterialScores) {
    // Starting position should have equal material for both sides
    int white_material = pos.get_material_score(Color::White);
    int black_material = pos.get_material_score(Color::Black);
    
    // Expected material: 8 pawns + 2 rooks + 2 knights + 2 bishops + 1 queen (excluding kings)
    // = 8*100 + 2*500 + 2*320 + 2*330 + 1*900 = 800 + 1000 + 640 + 660 + 900 = 4000
    int expected_material = 8*100 + 2*500 + 2*320 + 2*330 + 1*900;
    
    EXPECT_EQ(white_material, expected_material) << "White material should match expected value";
    EXPECT_EQ(black_material, expected_material) << "Black material should match expected value";
    EXPECT_EQ(pos.get_material_balance(), 0) << "Material balance should be 0 in starting position";
    EXPECT_EQ(pos.get_total_material(), 2 * expected_material) << "Total material should be twice the expected";
}

TEST_F(MaterialTrackingTest, CaptureUpdatesIncrementalMaterial) {
    // Remove the white pawn from d2 temporarily to avoid interference
    pos.set(sq(File::D, Rank::R2), Piece::None);
    
    // Place a black pawn on e4 that can be captured
    pos.set(sq(File::E, Rank::R4), make_piece(Color::Black, PieceType::Pawn));
    pos.rebuild_counts(); // Recalculate after manual piece placement
    
    // Save material scores after setup
    int white_material_after_setup = pos.get_material_score(Color::White);
    int black_material_after_setup = pos.get_material_score(Color::Black);
    
    // Place a white pawn on d2 to make the capture
    pos.set(sq(File::D, Rank::R2), make_piece(Color::White, PieceType::Pawn));
    pos.rebuild_counts();
    
    // Save material scores before capture
    int white_material_before = pos.get_material_score(Color::White);
    int black_material_before = pos.get_material_score(Color::Black);
    
    // Make a capture move: d2 pawn captures e4 pawn
    S_MOVE move = make_capture(sq(File::D, Rank::R2), sq(File::E, Rank::R4), PieceType::Pawn);
    
    // Make the move using VICE MakeMove function
    int move_result = pos.MakeMove(move);
    EXPECT_EQ(move_result, 1) << "Move should be legal";
    
    // After capture, black should lose the pawn value, white stays same
    EXPECT_EQ(pos.get_material_score(Color::Black), black_material_before - 100) 
        << "Black material should decrease by pawn value";
    EXPECT_EQ(pos.get_material_score(Color::White), white_material_before) 
        << "White material should remain unchanged";
    EXPECT_EQ(pos.get_material_balance(), white_material_before - (black_material_before - 100)) 
        << "Material balance should reflect the captured pawn";
    
    // Undo the move using VICE TakeMove function
    pos.TakeMove();
    
    // Verify material scores are restored
    EXPECT_EQ(pos.get_material_score(Color::Black), black_material_before) 
        << "Black material should be restored after undo";
    EXPECT_EQ(pos.get_material_score(Color::White), white_material_before) 
        << "White material should be restored after undo";
}

TEST_F(MaterialTrackingTest, PromotionUpdatesIncrementalMaterial) {
    // Clear some pieces to set up a promotion scenario
    // Remove pieces to make a clear path for pawn promotion
    for (int file = 0; file < 8; ++file) {
        pos.set(sq(static_cast<File>(file), Rank::R7), Piece::None); // Remove black pawns
        pos.set(sq(static_cast<File>(file), Rank::R8), Piece::None); // Remove black pieces
    }
    
    // Place a white pawn on the 7th rank ready for promotion
    pos.set(sq(File::E, Rank::R7), make_piece(Color::White, PieceType::Pawn));
    pos.rebuild_counts(); // Recalculate after manual setup
    
    int initial_white_material = pos.get_material_score(Color::White);
    int initial_black_material = pos.get_material_score(Color::Black);
    
    // Make a promotion move: e7-e8=Q
    S_MOVE move = make_promotion(sq(File::E, Rank::R7), sq(File::E, Rank::R8), PieceType::Queen);
    
    // Make the move using VICE MakeMove function
    int move_result = pos.MakeMove(move);
    EXPECT_EQ(move_result, 1) << "Move should be legal";
    
    // After promotion, white should lose pawn value and gain queen value
    int expected_material_change = value_of(make_piece(Color::White, PieceType::Queen)) - 
                                  value_of(make_piece(Color::White, PieceType::Pawn));
    EXPECT_EQ(pos.get_material_score(Color::White), initial_white_material + expected_material_change) 
        << "White material should increase by (Queen - Pawn) value";
    EXPECT_EQ(pos.get_material_score(Color::Black), initial_black_material) 
        << "Black material should remain unchanged";
    
    // Undo the move using VICE TakeMove function
    pos.TakeMove();
    
    // Verify material scores are restored
    EXPECT_EQ(pos.get_material_score(Color::White), initial_white_material) 
        << "White material should be restored after undo";
    EXPECT_EQ(pos.get_material_score(Color::Black), initial_black_material) 
        << "Black material should be restored after undo";
}

TEST_F(MaterialTrackingTest, MaterialConsistencyWithRebuildCounts) {
    // Make several moves and verify incremental material matches rebuild_counts
    std::vector<S_MOVE> moves = {
        make_move(sq(File::E, Rank::R2), sq(File::E, Rank::R4)),                    // e2-e4
        make_move(sq(File::D, Rank::R7), sq(File::D, Rank::R5)),                    // d7-d5
        make_capture(sq(File::E, Rank::R4), sq(File::D, Rank::R5), PieceType::Pawn), // exd5 (capture)
    };
    
    for (const auto& move : moves) {
        // Save incremental material before move
        int white_incremental = pos.get_material_score(Color::White);
        int black_incremental = pos.get_material_score(Color::Black);
        
        int move_result = pos.MakeMove(move);
        EXPECT_EQ(move_result, 1) << "Move should be legal";
        
        // Get incremental material after move
        int white_after_incremental = pos.get_material_score(Color::White);
        int black_after_incremental = pos.get_material_score(Color::Black);
        
        // Rebuild counts and check consistency
        pos.rebuild_counts();
        int white_after_rebuild = pos.get_material_score(Color::White);
        int black_after_rebuild = pos.get_material_score(Color::Black);
        
        EXPECT_EQ(white_after_incremental, white_after_rebuild) 
            << "White incremental material should match rebuild_counts";
        EXPECT_EQ(black_after_incremental, black_after_rebuild) 
            << "Black incremental material should match rebuild_counts";
    }
}
