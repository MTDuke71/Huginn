/**
 * @file test_king_lookup.cpp
 * @brief Google Test suite for king lookup table optimization
 */

#include <gtest/gtest.h>
#include "king_lookup_tables.hpp"
#include "king_optimizations.hpp"
#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "board120.hpp"
#include <chrono>
#include <vector>
#include <algorithm>

using namespace std::chrono;

/**
 * @brief Google Test fixture for king lookup table tests
 */
class KingLookupTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize lookup tables before each test
        KingLookupTables::initialize_king_tables();
    }
    
    /**
     * @brief Helper to extract and sort move destinations for comparison
     */
    std::vector<std::string> extract_moves(const S_MOVELIST& list) {
        std::vector<std::string> moves;
        for (int i = 0; i < list.count; ++i) {
            int from = (list.moves[i].move & MOVE_FROM_MASK) >> MOVE_FROM_SHIFT;
            int to = (list.moves[i].move & MOVE_TO_MASK) >> MOVE_TO_SHIFT;
            moves.push_back(std::to_string(from) + "-" + std::to_string(to));
        }
        std::sort(moves.begin(), moves.end());
        return moves;
    }
};

/**
 * @brief Test basic table initialization
 */
TEST_F(KingLookupTest, TableInitialization) {
    // Just test that initialization completed without errors
    // The lookup tables should be populated
    
    // Test a known position 
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("8/8/8/8/8/8/8/4K3 w - - 0 1"));
    
    S_MOVELIST moves;
    KingLookupTables::generate_king_moves_lookup(pos, moves, Color::White);
    
    // King on e1 should have exactly 5 moves (d1, f1, d2, e2, f2)
    EXPECT_EQ(moves.count, 5) << "King on e1 should have 5 moves";
}

/**
 * @brief Test correctness against template method for starting position
 */
TEST_F(KingLookupTest, StartingPositionCorrectness) {
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
    
    S_MOVELIST lookup_moves, template_moves;
    KingLookupTables::generate_king_moves_lookup(pos, lookup_moves, Color::White);
    KingOptimizations::generate_king_moves_optimized(pos, template_moves, Color::White);
    
    auto lookup_destinations = extract_moves(lookup_moves);
    auto template_destinations = extract_moves(template_moves);
    
    EXPECT_EQ(lookup_destinations, template_destinations) 
        << "Lookup and template methods should produce identical moves";
}

/**
 * @brief Test correctness for castling position
 */
TEST_F(KingLookupTest, CastlingPositionCorrectness) {
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"));
    
    S_MOVELIST lookup_moves, template_moves;
    KingLookupTables::generate_king_moves_lookup(pos, lookup_moves, Color::White);
    KingOptimizations::generate_king_moves_optimized(pos, template_moves, Color::White);
    
    auto lookup_destinations = extract_moves(lookup_moves);
    auto template_destinations = extract_moves(template_moves);
    
    EXPECT_EQ(lookup_destinations, template_destinations) 
        << "Castling moves should match between methods";
    EXPECT_GE(lookup_moves.count, 2) << "Should have king moves available";
}

/**
 * @brief Test boundary conditions (corner squares)
 */
TEST_F(KingLookupTest, BoundaryConditions) {
    std::vector<std::string> test_positions = {
        "k7/8/8/8/8/8/8/7K w - - 0 1",  // Corner kings
        "4k3/8/8/8/8/8/8/4K3 w - - 0 1",  // Edge kings
        "8/8/8/3k4/8/8/8/3K4 w - - 0 1"   // Center kings
    };
    
    for (const auto& fen : test_positions) {
        Position pos;
        ASSERT_TRUE(pos.set_from_fen(fen)) << "Failed to parse FEN: " << fen;
        
        S_MOVELIST lookup_moves, template_moves;
        KingLookupTables::generate_king_moves_lookup(pos, lookup_moves, Color::White);
        KingOptimizations::generate_king_moves_optimized(pos, template_moves, Color::White);
        
        auto lookup_destinations = extract_moves(lookup_moves);
        auto template_destinations = extract_moves(template_moves);
        
        EXPECT_EQ(lookup_destinations, template_destinations) 
            << "Boundary condition failed for: " << fen;
    }
}

/**
 * @brief Test performance (basic benchmark)
 */
TEST_F(KingLookupTest, PerformanceBenchmark) {
    Position pos;
    ASSERT_TRUE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
    
    const int iterations = 1000;
    
    // Time lookup method
    auto start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        S_MOVELIST moves;
        KingLookupTables::generate_king_moves_lookup(pos, moves, Color::White);
    }
    auto lookup_time = duration_cast<microseconds>(high_resolution_clock::now() - start);
    
    // Time template method
    start = high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        S_MOVELIST moves;
        KingOptimizations::generate_king_moves_optimized(pos, moves, Color::White);
    }
    auto template_time = duration_cast<microseconds>(high_resolution_clock::now() - start);
    
    // Just ensure lookup isn't drastically slower (allow 50% tolerance for debug builds)
    EXPECT_LE(lookup_time.count(), template_time.count() * 1.5) 
        << "Lookup method performance: " << lookup_time.count() 
        << "μs vs template: " << template_time.count() << "μs";
}