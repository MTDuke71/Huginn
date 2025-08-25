#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "sliding_piece_optimizations.hpp"
#include "move.hpp"

class SlidingPieceOptimizationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up common test positions
    }
    
    // Helper function to compare two move lists
    static bool compare_move_lists(const S_MOVELIST& list1, const S_MOVELIST& list2) {
        if (list1.count != list2.count) return false;
        
        std::vector<uint32_t> moves1, moves2;
        for (int i = 0; i < list1.count; ++i) {
            moves1.push_back(list1.moves[i].move);
        }
        for (int i = 0; i < list2.count; ++i) {
            moves2.push_back(list2.moves[i].move);
        }
        
        std::sort(moves1.begin(), moves1.end());
        std::sort(moves2.begin(), moves2.end());
        
        return moves1 == moves2;
    }
    
    // Test sliding piece moves for equivalence between original and optimized
    void test_sliding_piece_equivalence(const std::string& fen, const std::string& position_name) {
        Position pos;
        pos.set_from_fen(fen);
        
        // Generate moves using original functions
        S_MOVELIST original_bishop, original_rook, original_queen;
        generate_bishop_moves(pos, original_bishop, pos.side_to_move);
        generate_rook_moves(pos, original_rook, pos.side_to_move);
        generate_queen_moves(pos, original_queen, pos.side_to_move);
        
        // Test template-based optimizations
        S_MOVELIST opt_bishop, opt_rook, opt_queen;
        SlidingPieceOptimizations::generate_bishop_moves_optimized(pos, opt_bishop, pos.side_to_move);
        SlidingPieceOptimizations::generate_rook_moves_optimized(pos, opt_rook, pos.side_to_move);
        SlidingPieceOptimizations::generate_queen_moves_optimized(pos, opt_queen, pos.side_to_move);
        
        // Test macro-based optimizations
        S_MOVELIST macro_bishop, macro_rook, macro_queen;
        SlidingPieceOptimizations::generate_bishop_moves_macro(pos, macro_bishop, pos.side_to_move);
        SlidingPieceOptimizations::generate_rook_moves_macro(pos, macro_rook, pos.side_to_move);
        SlidingPieceOptimizations::generate_queen_moves_macro(pos, macro_queen, pos.side_to_move);
        
        // Verify equivalence
        EXPECT_TRUE(compare_move_lists(original_bishop, opt_bishop)) 
            << "Optimized bishop moves don't match original for: " << position_name;
        EXPECT_TRUE(compare_move_lists(original_rook, opt_rook)) 
            << "Optimized rook moves don't match original for: " << position_name;
        EXPECT_TRUE(compare_move_lists(original_queen, opt_queen)) 
            << "Optimized queen moves don't match original for: " << position_name;
            
        EXPECT_TRUE(compare_move_lists(original_bishop, macro_bishop)) 
            << "Macro bishop moves don't match original for: " << position_name;
        EXPECT_TRUE(compare_move_lists(original_rook, macro_rook)) 
            << "Macro rook moves don't match original for: " << position_name;
        EXPECT_TRUE(compare_move_lists(original_queen, macro_queen)) 
            << "Macro queen moves don't match original for: " << position_name;
            
        // Log move counts for verification
        std::cout << position_name << ": B=" << original_bishop.count 
                  << " R=" << original_rook.count 
                  << " Q=" << original_queen.count << " moves" << std::endl;
    }
};

// Test 1: Starting position sliding pieces
TEST_F(SlidingPieceOptimizationTest, StartingPosition) {
    test_sliding_piece_equivalence(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "Starting Position"
    );
}

// Test 2: Kiwipete position with active sliding pieces
TEST_F(SlidingPieceOptimizationTest, KiwipetePosition) {
    test_sliding_piece_equivalence(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "Kiwipete"
    );
}

// Test 3: Open position with long diagonals
TEST_F(SlidingPieceOptimizationTest, OpenPosition) {
    test_sliding_piece_equivalence(
        "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 0 1",
        "Open Position"
    );
}

// Test 4: Position with no sliding pieces (endgame)
TEST_F(SlidingPieceOptimizationTest, NoSlidingPieces) {
    test_sliding_piece_equivalence(
        "8/2k5/8/8/8/8/2K5/8 w - - 0 1",
        "No Sliding Pieces"
    );
}

// Test 5: Queen vs pawns endgame
TEST_F(SlidingPieceOptimizationTest, QueenEndgame) {
    test_sliding_piece_equivalence(
        "8/8/8/8/8/2k5/8/2K1Q3 w - - 0 1",
        "Queen Endgame"
    );
}

// Test 6: Rook endgame
TEST_F(SlidingPieceOptimizationTest, RookEndgame) {
    test_sliding_piece_equivalence(
        "8/8/8/8/8/2k5/8/2KR4 w - - 0 1",
        "Rook Endgame"
    );
}

// Test 7: Bishop pair vs knight
TEST_F(SlidingPieceOptimizationTest, BishopPair) {
    test_sliding_piece_equivalence(
        "8/8/8/3n4/8/2k5/8/2KBB3 w - - 0 1",
        "Bishop Pair"
    );
}

// Test 8: Complex tactical position
TEST_F(SlidingPieceOptimizationTest, TacticalPosition) {
    test_sliding_piece_equivalence(
        "r1bq1rk1/pp2ppbp/2np1np1/8/3PP3/2N1BN2/PPP1BPPP/R2QK2R w KQ - 0 1",
        "Tactical Position"
    );
}

// Test 9: Sliding pieces blocked by pawns
TEST_F(SlidingPieceOptimizationTest, BlockedSlidingPieces) {
    test_sliding_piece_equivalence(
        "r1bqkb1r/pppppppp/2n2n2/8/8/2N2N2/PPPPPPPP/R1BQKB1R w KQkq - 0 1",
        "Blocked Sliding Pieces"
    );
}

// Test 10: Black sliding pieces
TEST_F(SlidingPieceOptimizationTest, BlackSlidingPieces) {
    test_sliding_piece_equivalence(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1",
        "Black Sliding Pieces"
    );
}

// Performance comparison test
TEST_F(SlidingPieceOptimizationTest, PerformanceComparison) {
    std::vector<std::string> test_positions = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 0 1",
        "r1bq1rk1/pp2ppbp/2np1np1/8/3PP3/2N1BN2/PPP1BPPP/R2QK2R w KQ - 0 1",
        "8/8/8/8/8/2k5/8/2K1Q3 w - - 0 1"
    };
    
    const int iterations = 5000;
    
    // Test original implementations
    auto start_original = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST bishop_list, rook_list, queen_list;
            generate_bishop_moves(pos, bishop_list, pos.side_to_move);
            generate_rook_moves(pos, rook_list, pos.side_to_move);
            generate_queen_moves(pos, queen_list, pos.side_to_move);
        }
    }
    auto end_original = std::chrono::high_resolution_clock::now();
    
    // Test template-based optimizations
    auto start_template = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST bishop_list, rook_list, queen_list;
            SlidingPieceOptimizations::generate_bishop_moves_optimized(pos, bishop_list, pos.side_to_move);
            SlidingPieceOptimizations::generate_rook_moves_optimized(pos, rook_list, pos.side_to_move);
            SlidingPieceOptimizations::generate_queen_moves_optimized(pos, queen_list, pos.side_to_move);
        }
    }
    auto end_template = std::chrono::high_resolution_clock::now();
    
    // Test macro-based optimizations
    auto start_macro = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST bishop_list, rook_list, queen_list;
            SlidingPieceOptimizations::generate_bishop_moves_macro(pos, bishop_list, pos.side_to_move);
            SlidingPieceOptimizations::generate_rook_moves_macro(pos, rook_list, pos.side_to_move);
            SlidingPieceOptimizations::generate_queen_moves_macro(pos, queen_list, pos.side_to_move);
        }
    }
    auto end_macro = std::chrono::high_resolution_clock::now();
    
    // Test combined optimization
    auto start_combined = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST list;
            SlidingPieceOptimizations::generate_all_sliding_moves_optimized(pos, list, pos.side_to_move);
        }
    }
    auto end_combined = std::chrono::high_resolution_clock::now();
    
    // Calculate durations
    auto original_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_original - start_original);
    auto template_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_template - start_template);
    auto macro_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_macro - start_macro);
    auto combined_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_combined - start_combined);
    
    // Report results
    std::cout << "\n=== Sliding Piece Move Generation Performance ===" << std::endl;
    std::cout << "Original implementation:  " << original_duration.count() << " μs" << std::endl;
    std::cout << "Template implementation:  " << template_duration.count() << " μs" << std::endl;
    std::cout << "Macro implementation:     " << macro_duration.count() << " μs" << std::endl;
    std::cout << "Combined implementation:  " << combined_duration.count() << " μs" << std::endl;
    
    // Calculate improvements
    if (original_duration.count() > 0) {
        auto calc_improvement = [&](auto duration) -> double {
            return (double(original_duration.count()) / double(duration.count()) - 1.0) * 100.0;
        };
        
        std::cout << "\nPerformance improvements:" << std::endl;
        std::cout << "Template: " << std::fixed << std::setprecision(1) << calc_improvement(template_duration) << "%" << std::endl;
        std::cout << "Macro:    " << std::fixed << std::setprecision(1) << calc_improvement(macro_duration) << "%" << std::endl;
        std::cout << "Combined: " << std::fixed << std::setprecision(1) << calc_improvement(combined_duration) << "%" << std::endl;
    }
}

// Test early exit optimization for positions with no sliding pieces
TEST_F(SlidingPieceOptimizationTest, EarlyExitOptimization) {
    Position pos;
    pos.set_from_fen("8/2k5/8/8/8/8/2K5/8 w - - 0 1");  // No sliding pieces
    
    S_MOVELIST original_list, optimized_list;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 50000; ++i) {
        generate_bishop_moves(pos, original_list, pos.side_to_move);
        generate_rook_moves(pos, original_list, pos.side_to_move);
        generate_queen_moves(pos, original_list, pos.side_to_move);
        original_list.count = 0;  // Reset for next iteration
    }
    auto mid = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 50000; ++i) {
        SlidingPieceOptimizations::generate_all_sliding_moves_optimized(pos, optimized_list, pos.side_to_move);
        optimized_list.count = 0;  // Reset for next iteration
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto original_time = std::chrono::duration_cast<std::chrono::microseconds>(mid - start);
    auto optimized_time = std::chrono::duration_cast<std::chrono::microseconds>(end - mid);
    
    std::cout << "No-sliding-pieces position (50k iterations):" << std::endl;
    std::cout << "Original: " << original_time.count() << " μs" << std::endl;
    std::cout << "Optimized: " << optimized_time.count() << " μs" << std::endl;
    
    if (original_time.count() > 0) {
        double improvement = (double(original_time.count()) / double(optimized_time.count()) - 1.0) * 100.0;
        std::cout << "Early exit improvement: " << std::fixed << std::setprecision(1) << improvement << "%" << std::endl;
    }
}

// Test combined sliding piece generation
TEST_F(SlidingPieceOptimizationTest, CombinedGeneration) {
    Position pos;
    pos.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    
    // Generate using individual functions
    S_MOVELIST individual_list;
    SlidingPieceOptimizations::generate_bishop_moves_optimized(pos, individual_list, pos.side_to_move);
    SlidingPieceOptimizations::generate_rook_moves_optimized(pos, individual_list, pos.side_to_move);
    SlidingPieceOptimizations::generate_queen_moves_optimized(pos, individual_list, pos.side_to_move);
    
    // Generate using combined function
    S_MOVELIST combined_list;
    SlidingPieceOptimizations::generate_all_sliding_moves_optimized(pos, combined_list, pos.side_to_move);
    
    // Should be identical
    EXPECT_TRUE(compare_move_lists(individual_list, combined_list))
        << "Combined generation doesn't match individual functions";
        
    std::cout << "Combined generation: " << combined_list.count << " sliding piece moves" << std::endl;
}
