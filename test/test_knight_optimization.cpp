#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "knight_optimizations.hpp"
#include "move.hpp"

class KnightOptimizationTest : public ::testing::Test {
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
    
    // Test knight moves for equivalence between original and optimized
    void test_knight_move_equivalence(const std::string& fen, const std::string& position_name) {
        Position pos;
        pos.set_from_fen(fen);
        
        // Generate moves using original function
        S_MOVELIST original_list;
        generate_knight_moves(pos, original_list, pos.side_to_move);
        
        // Test all optimization variants
        S_MOVELIST unrolled_list, macro_list, template_list;
        
        KnightOptimizations::generate_knight_moves_optimized(pos, unrolled_list, pos.side_to_move);
        KnightOptimizations::generate_knight_moves_macro(pos, macro_list, pos.side_to_move);
        KnightOptimizations::generate_knight_moves_template(pos, template_list, pos.side_to_move);
        
        // They should all be identical
        EXPECT_TRUE(compare_move_lists(original_list, unrolled_list)) 
            << "Unrolled knight moves don't match original for: " << position_name;
        EXPECT_TRUE(compare_move_lists(original_list, macro_list)) 
            << "Macro knight moves don't match original for: " << position_name;
        EXPECT_TRUE(compare_move_lists(original_list, template_list)) 
            << "Template knight moves don't match original for: " << position_name;
            
        // Log move counts for verification
        std::cout << position_name << ": " << original_list.count << " knight moves" << std::endl;
    }
};

// Test 1: Starting position knight moves
TEST_F(KnightOptimizationTest, StartingPositionKnightMoves) {
    test_knight_move_equivalence(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "Starting Position"
    );
}

// Test 2: Kiwipete position with multiple knights
TEST_F(KnightOptimizationTest, KiwipetePosition) {
    test_knight_move_equivalence(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "Kiwipete"
    );
}

// Test 3: Position with knights in center
TEST_F(KnightOptimizationTest, KnightsInCenter) {
    test_knight_move_equivalence(
        "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 0 1",
        "Knights in Center"
    );
}

// Test 4: Position with no knights (endgame)
TEST_F(KnightOptimizationTest, NoKnights) {
    test_knight_move_equivalence(
        "8/2k5/8/8/8/8/2K5/8 w - - 0 1",
        "No Knights"
    );
}

// Test 5: Position with knight on edge
TEST_F(KnightOptimizationTest, KnightOnEdge) {
    test_knight_move_equivalence(
        "8/8/8/8/8/8/8/N6k w - - 0 1",
        "Knight on Edge"
    );
}

// Test 6: Position with knight in corner
TEST_F(KnightOptimizationTest, KnightInCorner) {
    test_knight_move_equivalence(
        "N7/8/8/8/8/8/8/7k w - - 0 1",
        "Knight in Corner"
    );
}

// Test 7: Tactical position with many knights
TEST_F(KnightOptimizationTest, TacticalPosition) {
    test_knight_move_equivalence(
        "r1bqk2r/pp2nppp/2n1p3/3pP3/2pP4/2N1BN2/PP3PPP/R2QKB1R w KQkq - 0 1",
        "Tactical Position"
    );
}

// Test 8: Knight blocked by own pieces
TEST_F(KnightOptimizationTest, KnightBlocked) {
    test_knight_move_equivalence(
        "8/8/2PPP3/2PNP3/2PPP3/8/8/7k w - - 0 1",
        "Knight Blocked"
    );
}

// Test 9: Knight with capture opportunities
TEST_F(KnightOptimizationTest, KnightWithCaptures) {
    test_knight_move_equivalence(
        "8/8/2ppp3/2pNp3/2ppp3/8/8/7k w - - 0 1",
        "Knight with Captures"
    );
}

// Test 10: Black knight moves
TEST_F(KnightOptimizationTest, BlackKnightMoves) {
    test_knight_move_equivalence(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1",
        "Black Knight Moves"
    );
}

// Performance comparison test
TEST_F(KnightOptimizationTest, PerformanceComparison) {
    std::vector<std::string> test_positions = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 0 1",
        "r1bqk2r/pp2nppp/2n1p3/3pP3/2pP4/2N1BN2/PP3PPP/R2QKB1R w KQkq - 0 1",
        "8/8/2ppp3/2pNp3/2ppp3/8/8/7k w - - 0 1"
    };
    
    const int iterations = 10000;
    
    // Test original implementation
    auto start_original = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST list;
            generate_knight_moves(pos, list, pos.side_to_move);
        }
    }
    auto end_original = std::chrono::high_resolution_clock::now();
    
    // Test unrolled implementation
    auto start_unrolled = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST list;
            KnightOptimizations::generate_knight_moves_optimized(pos, list, pos.side_to_move);
        }
    }
    auto end_unrolled = std::chrono::high_resolution_clock::now();
    
    // Test macro implementation
    auto start_macro = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST list;
            KnightOptimizations::generate_knight_moves_macro(pos, list, pos.side_to_move);
        }
    }
    auto end_macro = std::chrono::high_resolution_clock::now();
    
    // Test template implementation
    auto start_template = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST list;
            KnightOptimizations::generate_knight_moves_template(pos, list, pos.side_to_move);
        }
    }
    auto end_template = std::chrono::high_resolution_clock::now();
    
    // Calculate durations
    auto original_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_original - start_original);
    auto unrolled_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_unrolled - start_unrolled);
    auto macro_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_macro - start_macro);
    auto template_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_template - start_template);
    
    // Report results
    std::cout << "\n=== Knight Move Generation Performance ===" << std::endl;
    std::cout << "Original implementation:  " << original_duration.count() << " μs" << std::endl;
    std::cout << "Unrolled implementation:  " << unrolled_duration.count() << " μs" << std::endl;
    std::cout << "Macro implementation:     " << macro_duration.count() << " μs" << std::endl;
    std::cout << "Template implementation:  " << template_duration.count() << " μs" << std::endl;
    
    // Calculate improvements
    if (original_duration.count() > 0) {
        auto calc_improvement = [&](auto duration) -> double {
            return (double(original_duration.count()) / double(duration.count()) - 1.0) * 100.0;
        };
        
        std::cout << "\nPerformance improvements:" << std::endl;
        std::cout << "Unrolled: " << std::fixed << std::setprecision(1) << calc_improvement(unrolled_duration) << "%" << std::endl;
        std::cout << "Macro:    " << std::fixed << std::setprecision(1) << calc_improvement(macro_duration) << "%" << std::endl;
        std::cout << "Template: " << std::fixed << std::setprecision(1) << calc_improvement(template_duration) << "%" << std::endl;
    }
}

// Test early exit optimization
TEST_F(KnightOptimizationTest, EarlyExitOptimization) {
    // Position with no knights should have very fast generation
    Position pos;
    pos.set_from_fen("8/2k5/8/8/8/8/2K5/8 w - - 0 1");
    
    S_MOVELIST original_list, optimized_list;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; ++i) {
        generate_knight_moves(pos, original_list, pos.side_to_move);
        original_list.count = 0;  // Reset for next iteration
    }
    auto mid = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100000; ++i) {
        KnightOptimizations::generate_knight_moves_optimized(pos, optimized_list, pos.side_to_move);
        optimized_list.count = 0;  // Reset for next iteration
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto original_time = std::chrono::duration_cast<std::chrono::microseconds>(mid - start);
    auto optimized_time = std::chrono::duration_cast<std::chrono::microseconds>(end - mid);
    
    std::cout << "No-knights position (100k iterations):" << std::endl;
    std::cout << "Original: " << original_time.count() << " μs" << std::endl;
    std::cout << "Optimized: " << optimized_time.count() << " μs" << std::endl;
    
    // Both should generate 0 moves
    EXPECT_EQ(0, original_list.count);
    EXPECT_EQ(0, optimized_list.count);
}
