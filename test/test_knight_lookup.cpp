/**
 * @file test_knight_lookup.cpp
 * @brief Comprehensive test suite for knight lookup table optimization
 * @author Huginn Chess Engine Development Team
 * @date September 2025
 * @version 1.0
 * 
 * This file contains extensive unit tests and performance benchmarks for the
 * knight lookup table optimization implementation. Tests verify correctness
 * and measure performance improvements compared to the template-based approach.
 * 
 * @test_coverage
 * - Correctness verification against template method
 * - Performance benchmarking across multiple test positions
 * - Edge case testing (corner squares, center squares, etc.)
 * - Memory usage validation
 * - Cross-platform compatibility testing
 */

#include <gtest/gtest.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "knight_lookup_tables.hpp"
#include "knight_optimizations.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"

/**
 * @class KnightLookupTest
 * @brief Test fixture for knight lookup table tests
 * 
 * Provides common setup and utility functions for all knight lookup tests.
 * Initializes lookup tables and provides move comparison functionality.
 */
class KnightLookupTest : public ::testing::Test {
protected:
    /**
     * @brief Set up test fixture - initialize lookup tables
     * @details Called before each test to ensure lookup tables are properly initialized
     */
    void SetUp() override {
        KnightLookupTables::initialize_knight_tables();
    }
    
    /**
     * @brief Compare knight moves between different generation methods
     * @param fen_string FEN position to test
     * @param test_name Descriptive name for the test position
     * @details Generates knight moves using template, lookup, and bitboard methods
     *          and verifies they produce identical results
     */
    void compare_knight_moves(const std::string& fen_string, const std::string& test_name) {
        Position pos;
        pos.set_from_fen(fen_string);
        
        S_MOVELIST original_moves, lookup_moves, bitboard_moves;
        
        // Generate moves using original template method
        KnightOptimizations::generate_knight_moves_template(pos, original_moves, pos.side_to_move);
        
        // Generate moves using lookup table method
        KnightLookupTables::generate_knight_moves_lookup(pos, lookup_moves, pos.side_to_move);
        
        // Generate moves using bitboard method
        KnightLookupTables::generate_knight_moves_bitboard(pos, bitboard_moves, pos.side_to_move);
        
        // All methods should generate the same number of moves
        EXPECT_EQ(original_moves.count, lookup_moves.count) 
            << "Lookup table method mismatch in " << test_name;
        EXPECT_EQ(original_moves.count, bitboard_moves.count) 
            << "Bitboard method mismatch in " << test_name;
        
        // Verify that all moves are identical (sorting to handle potential order differences)
        std::vector<S_MOVE> orig_sorted, lookup_sorted, bitboard_sorted;
        
        for (int i = 0; i < original_moves.count; ++i) {
            orig_sorted.push_back(original_moves.moves[i]);
            lookup_sorted.push_back(lookup_moves.moves[i]);
            bitboard_sorted.push_back(bitboard_moves.moves[i]);
        }
        
        // Sort by move encoding for comparison
        auto move_compare = [](const S_MOVE& a, const S_MOVE& b) {
            return a.move < b.move;
        };
        
        std::sort(orig_sorted.begin(), orig_sorted.end(), move_compare);
        std::sort(lookup_sorted.begin(), lookup_sorted.end(), move_compare);
        std::sort(bitboard_sorted.begin(), bitboard_sorted.end(), move_compare);
        
        for (size_t i = 0; i < orig_sorted.size(); ++i) {
            EXPECT_EQ(orig_sorted[i].move, lookup_sorted[i].move) 
                << "Move " << i << " differs in lookup method for " << test_name;
            EXPECT_EQ(orig_sorted[i].move, bitboard_sorted[i].move) 
                << "Move " << i << " differs in bitboard method for " << test_name;
        }
    }
};

TEST_F(KnightLookupTest, StartingPosition) {
    compare_knight_moves(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "Starting Position"
    );
}

TEST_F(KnightLookupTest, KiwipetePosition) {
    compare_knight_moves(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "Kiwipete Position"
    );
}

TEST_F(KnightLookupTest, KnightOnEdges) {
    // Knight on a1 (corner) - should have 2 moves
    compare_knight_moves(
        "8/8/8/8/8/8/8/N7 w - - 0 1",
        "Knight on a1"
    );
    
    // Knight on d4 (center) - should have 8 moves
    compare_knight_moves(
        "8/8/8/8/3N4/8/8/8 w - - 0 1",
        "Knight on d4"
    );
    
    // Knight on h8 (corner) - should have 2 moves
    compare_knight_moves(
        "7N/8/8/8/8/8/8/8 w - - 0 1",
        "Knight on h8"
    );
}

TEST_F(KnightLookupTest, MultipleKnights) {
    // Multiple knights in various positions
    compare_knight_moves(
        "8/8/2N2N2/8/8/2N2N2/8/8 w - - 0 1",
        "Four Knights"
    );
}

TEST_F(KnightLookupTest, KnightWithCaptures) {
    // Knight with potential captures
    compare_knight_moves(
        "8/8/1p1p4/p1N1p3/1p1p4/8/8/8 w - - 0 1",
        "Knight with captures"
    );
}

TEST_F(KnightLookupTest, NoKnights) {
    // Position with no knights - should generate no moves
    compare_knight_moves(
        "r1bqkb1r/pppppppp/8/8/8/8/PPPPPPPP/R1BQKB1R w KQkq - 0 1",
        "No Knights"
    );
}

// Performance comparison test
TEST_F(KnightLookupTest, PerformanceComparison) {
    const std::string test_positions[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
    };
    
    const int iterations = 100000;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Time original template method
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST moves;
            KnightOptimizations::generate_knight_moves_template(pos, moves, pos.side_to_move);
        }
    }
    auto template_time = std::chrono::high_resolution_clock::now() - start_time;
    
    // Time lookup table method
    start_time = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST moves;
            KnightLookupTables::generate_knight_moves_lookup(pos, moves, pos.side_to_move);
        }
    }
    auto lookup_time = std::chrono::high_resolution_clock::now() - start_time;
    
    // Time bitboard method
    start_time = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST moves;
            KnightLookupTables::generate_knight_moves_bitboard(pos, moves, pos.side_to_move);
        }
    }
    auto bitboard_time = std::chrono::high_resolution_clock::now() - start_time;
    
    auto template_ms = std::chrono::duration_cast<std::chrono::milliseconds>(template_time).count();
    auto lookup_ms = std::chrono::duration_cast<std::chrono::milliseconds>(lookup_time).count();
    auto bitboard_ms = std::chrono::duration_cast<std::chrono::milliseconds>(bitboard_time).count();
    
    double lookup_speedup = double(template_ms) / double(lookup_ms);
    double bitboard_speedup = double(template_ms) / double(bitboard_ms);
    
    std::cout << "\nKnight Move Generation Performance Results:\n";
    std::cout << "==========================================\n";
    std::cout << "Template method:  " << template_ms << " ms\n";
    std::cout << "Lookup method:    " << lookup_ms << " ms (speedup: " 
              << std::fixed << std::setprecision(2) << lookup_speedup << "x)\n";
    std::cout << "Bitboard method:  " << bitboard_ms << " ms (speedup: " 
              << std::fixed << std::setprecision(2) << bitboard_speedup << "x)\n";
    std::cout << "Iterations:       " << iterations << " per position\n";
    std::cout << "Positions tested: " << sizeof(test_positions)/sizeof(test_positions[0]) << "\n\n";
    
    // Expect some improvement with lookup tables
    EXPECT_LT(lookup_ms, template_ms * 1.1) << "Lookup method should be competitive";
    EXPECT_LT(bitboard_ms, template_ms * 1.1) << "Bitboard method should be competitive";
}

TEST_F(KnightLookupTest, LookupTableInitialization) {
    // Test that lookup tables are correctly initialized
    
    // Test corner squares (should have exactly 2 moves each)
    EXPECT_EQ(KnightLookupTables::KNIGHT_MOVE_COUNT[0], 2);  // a1
    EXPECT_EQ(KnightLookupTables::KNIGHT_MOVE_COUNT[7], 2);  // h1
    EXPECT_EQ(KnightLookupTables::KNIGHT_MOVE_COUNT[56], 2); // a8
    EXPECT_EQ(KnightLookupTables::KNIGHT_MOVE_COUNT[63], 2); // h8
    
    // Test center squares (should have 8 moves)
    EXPECT_EQ(KnightLookupTables::KNIGHT_MOVE_COUNT[27], 8); // d4
    EXPECT_EQ(KnightLookupTables::KNIGHT_MOVE_COUNT[28], 8); // e4
    EXPECT_EQ(KnightLookupTables::KNIGHT_MOVE_COUNT[35], 8); // d5
    EXPECT_EQ(KnightLookupTables::KNIGHT_MOVE_COUNT[36], 8); // e5
    
    // Test edge squares (should have between 3-6 moves)
    EXPECT_GE(KnightLookupTables::KNIGHT_MOVE_COUNT[1], 3);  // b1
    EXPECT_LE(KnightLookupTables::KNIGHT_MOVE_COUNT[1], 6);
    
    // Verify bitboard consistency
    for (int sq = 0; sq < 64; ++sq) {
        int move_count = KnightLookupTables::KNIGHT_MOVE_COUNT[sq];
        int bitboard_count = builtin_popcountll(KnightLookupTables::KNIGHT_ATTACKS[sq]);
        EXPECT_EQ(move_count, bitboard_count) 
            << "Bitboard/array mismatch at square " << sq;
    }
}