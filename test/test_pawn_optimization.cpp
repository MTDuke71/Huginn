#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "pawn_optimizations.hpp"

class PawnOptimizationTest : public ::testing::Test {
protected:
    void SetUp() override {
        pos.reset();
    }
    
    Position pos;
    S_MOVELIST original_moves;
    S_MOVELIST optimized_moves;
    
    // Helper to compare two move lists for equality (ignoring order)
    bool compare_move_lists(const S_MOVELIST& list1, const S_MOVELIST& list2) {
        if (list1.count != list2.count) {
            return false;
        }
        
        // Create sorted copies for comparison
        std::vector<S_MOVE> moves1(list1.moves, list1.moves + list1.count);
        std::vector<S_MOVE> moves2(list2.moves, list2.moves + list2.count);
        
        // Sort by move encoding (from, to, flags)
        auto move_comparator = [](const S_MOVE& a, const S_MOVE& b) {
            return a.move < b.move;
        };
        
        std::sort(moves1.begin(), moves1.end(), move_comparator);
        std::sort(moves2.begin(), moves2.end(), move_comparator);
        
        // Compare each move
        for (size_t i = 0; i < moves1.size(); ++i) {
            if (moves1[i].move != moves2[i].move) {
                return false;
            }
        }
        
        return true;
    }
    
    void test_position_equivalence(const std::string& fen, const std::string& description) {
        pos.set_from_fen(fen);
        
        // Generate moves with original implementation
        original_moves.count = 0;
        generate_pawn_moves(pos, original_moves, pos.side_to_move);
        
        // Generate moves with optimized implementation  
        optimized_moves.count = 0;
        PawnOptimizations::generate_pawn_moves_optimized(pos, optimized_moves, pos.side_to_move);
        
        // Compare results
        EXPECT_TRUE(compare_move_lists(original_moves, optimized_moves)) 
            << "Move lists differ for position: " << description << " (" << fen << ")"
            << "\nOriginal count: " << original_moves.count
            << "\nOptimized count: " << optimized_moves.count;
    }
};

TEST_F(PawnOptimizationTest, StartingPosition) {
    test_position_equivalence(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "Starting Position"
    );
}

TEST_F(PawnOptimizationTest, PromotionPosition) {
    test_position_equivalence(
        "8/P6P/8/8/8/8/p6p/8 w - - 0 1",
        "Promotion Position"
    );
}

TEST_F(PawnOptimizationTest, PromotionWithCaptures) {
    test_position_equivalence(
        "1nbqkbn1/P6P/8/8/8/8/p6p/1NBQKBN1 w - - 0 1",
        "Promotion with Captures"
    );
}

TEST_F(PawnOptimizationTest, EnPassantPosition) {
    test_position_equivalence(
        "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
        "En Passant Position"
    );
}

TEST_F(PawnOptimizationTest, MidgamePosition) {
    test_position_equivalence(
        "r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
        "Midgame Position"
    );
}

TEST_F(PawnOptimizationTest, ComplexPromotionScenario) {
    test_position_equivalence(
        "4k3/P1P1P1P1/8/8/8/8/p1p1p1p1/4K3 w - - 0 1",
        "Complex Promotion Scenario"
    );
}

TEST_F(PawnOptimizationTest, BlackToMove) {
    test_position_equivalence(
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "Black to Move"
    );
}

TEST_F(PawnOptimizationTest, BlackPromotions) {
    test_position_equivalence(
        "4k3/8/8/8/8/8/p1p1p1p1/R1B1KB1R b - - 0 1",
        "Black Promotions"
    );
}

TEST_F(PawnOptimizationTest, Nopawns) {
    test_position_equivalence(
        "rnbqkbnr/8/8/8/8/8/8/RNBQKBNR w KQkq - 0 1",
        "No Pawns"
    );
}

// Test move scoring consistency
TEST_F(PawnOptimizationTest, PromotionScoring) {
    pos.set_from_fen("8/P7/8/8/8/8/8/8 w - - 0 1");
    
    // Generate with optimized version
    optimized_moves.count = 0;
    PawnOptimizations::generate_pawn_moves_optimized(pos, optimized_moves, pos.side_to_move);
    
    EXPECT_EQ(optimized_moves.count, 4); // 4 promotion moves
    
    // Verify all moves are promotions with proper scoring
    bool found_queen = false, found_rook = false, found_bishop = false, found_knight = false;
    
    for (int i = 0; i < optimized_moves.count; ++i) {
        const S_MOVE& move = optimized_moves.moves[i];
        EXPECT_TRUE(move.is_promotion()) << "Move " << i << " should be a promotion";
        EXPECT_GT(move.score, 2000000) << "Promotion moves should have high scores";
        
        switch (move.get_promoted()) {
            case PieceType::Queen:  found_queen = true; break;
            case PieceType::Rook:   found_rook = true; break;
            case PieceType::Bishop: found_bishop = true; break;
            case PieceType::Knight: found_knight = true; break;
            default: FAIL() << "Invalid promotion piece type";
        }
    }
    
    EXPECT_TRUE(found_queen && found_rook && found_bishop && found_knight)
        << "Should generate all 4 promotion types";
}

// Test batch generation performance characteristics
TEST_F(PawnOptimizationTest, PromotionBatchGeneration) {
    S_MOVELIST test_list;
    test_list.count = 0;
    
    // Test the batch generation function directly
    PawnOptimizations::generate_promotion_batch(test_list, sq(File::A, Rank::R7), sq(File::A, Rank::R8));
    
    EXPECT_EQ(test_list.count, 4);
    
    // Verify all promotions are generated correctly
    std::set<PieceType> promotion_types;
    for (int i = 0; i < test_list.count; ++i) {
        const S_MOVE& move = test_list.moves[i];
        EXPECT_EQ(move.get_from(), sq(File::A, Rank::R7));
        EXPECT_EQ(move.get_to(), sq(File::A, Rank::R8));
        EXPECT_TRUE(move.is_promotion());
        EXPECT_FALSE(move.is_capture());
        promotion_types.insert(move.get_promoted());
    }
    
    EXPECT_EQ(promotion_types.size(), 4) << "Should generate 4 unique promotion types";
}

// Test capture-promotion batch generation
TEST_F(PawnOptimizationTest, CapturePromotionBatchGeneration) {
    S_MOVELIST test_list;
    test_list.count = 0;
    
    // Test capture-promotion batch generation
    PawnOptimizations::generate_promotion_batch(test_list, sq(File::A, Rank::R7), sq(File::B, Rank::R8), PieceType::Rook);
    
    EXPECT_EQ(test_list.count, 4);
    
    // Verify all capture-promotions are generated correctly
    for (int i = 0; i < test_list.count; ++i) {
        const S_MOVE& move = test_list.moves[i];
        EXPECT_EQ(move.get_from(), sq(File::A, Rank::R7));
        EXPECT_EQ(move.get_to(), sq(File::B, Rank::R8));
        EXPECT_TRUE(move.is_promotion());
        EXPECT_TRUE(move.is_capture());
        EXPECT_EQ(move.get_captured(), PieceType::Rook);
        EXPECT_GT(move.score, 2000000) << "Capture-promotions should have high scores";
    }
}
