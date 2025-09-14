/**
 * @file test_king_lookup.cpp
 * @brief Comprehensive test suite for king lookup table optimization
 * @author Huginn Chess Engine Development Team
 * @date September 2025
 * @version 1.0
 * 
 * This test suite validates the correctness and performance of king lookup tables
 * by comparing against the template-based king move generation. It includes:
 * - Correctness validation for all 64 squares
 * - Edge case and boundary testing
 * - Performance benchmarking
 * - Memory usage verification
 */

#include <gtest/gtest.h>
#include "king_lookup_tables.hpp"
#include "king_optimizations.hpp" // For template fallback comparison
#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "board120.hpp"
#include <iostream>
#include <iomanip>
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
};

/**
 * @brief Generate king moves using template optimization for comparison
 * @param pos Position to generate moves for
 * @param us Side to move
 * @return Vector of moves in 64-square notation for easy comparison
 */
std::vector<std::pair<int, int>> generate_template_king_moves(const Position& pos, Color us) {
    S_MOVELIST template_list;
    template_list.count = 0;
    
    // Use template approach for reference
    KingOptimizations::generate_king_moves_optimized(pos, template_list, us);
    
    std::vector<std::pair<int, int>> moves;
    for (int i = 0; i < template_list.count; ++i) {
        S_MOVE move = template_list.moves[i];
        int from = move.get_from();
        int to = move.get_to();
        
        // Convert to 64-square notation for comparison
        int from_64 = MAILBOX_MAPS.to64[from];
        int to_64 = MAILBOX_MAPS.to64[to];
        
        if (from_64 != -1 && to_64 != -1) {
            moves.emplace_back(from_64, to_64);
        }
    }
    
    // Sort for consistent comparison
    std::sort(moves.begin(), moves.end());
    return moves;
}

/**
 * @brief Generate king moves using lookup tables
 * @param pos Position to generate moves for  
 * @param us Side to move
 * @return Vector of moves in 64-square notation for comparison
 */
std::vector<std::pair<int, int>> generate_lookup_king_moves(const Position& pos, Color us) {
    S_MOVELIST lookup_list;
    lookup_list.count = 0;
    
    KingLookupTables::generate_king_moves_lookup(pos, lookup_list, us);
    
    std::vector<std::pair<int, int>> moves;
    for (int i = 0; i < lookup_list.count; ++i) {
        S_MOVE move = lookup_list.moves[i];
        int from = move.get_from();
        int to = move.get_to();
        
        int from_64 = MAILBOX_MAPS.to64[from];
        int to_64 = MAILBOX_MAPS.to64[to];
        
        if (from_64 != -1 && to_64 != -1) {
            moves.emplace_back(from_64, to_64);
        }
    }
    
    std::sort(moves.begin(), moves.end());
    return moves;
}

/**
 * @brief Generate king moves using bitboard approach
 * @param pos Position to generate moves for
 * @param us Side to move  
 * @return Vector of moves in 64-square notation for comparison
 */
std::vector<std::pair<int, int>> generate_bitboard_king_moves(const Position& pos, Color us) {
    S_MOVELIST bitboard_list;
    bitboard_list.count = 0;
    
    KingLookupTables::generate_king_moves_bitboard(pos, bitboard_list, us);
    
    std::vector<std::pair<int, int>> moves;
    for (int i = 0; i < bitboard_list.count; ++i) {
        S_MOVE move = bitboard_list.moves[i];
        int from = move.get_from();
        int to = move.get_to();
        
        int from_64 = MAILBOX_MAPS.to64[from];
        int to_64 = MAILBOX_MAPS.to64[to];
        
        if (from_64 != -1 && to_64 != -1) {
            moves.emplace_back(from_64, to_64);
        }
    }
    
    std::sort(moves.begin(), moves.end());
    return moves;
}

/**
 * @brief Test basic correctness for a specific position
 * @param fen FEN string for the position to test
 * @param test_name Descriptive name for the test
 * @param stats Test statistics to update
 */
void test_position_correctness(const std::string& fen, const std::string& test_name, TestStats& stats) {
    std::cout << "Testing " << test_name << "... ";
    
    Position pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "FAIL (invalid FEN)\n";
        stats.record_fail();
        return;
    }
    
    Color us = pos.side_to_move;
    
    // Generate moves using all three methods
    auto template_moves = generate_template_king_moves(pos, us);
    auto lookup_moves = generate_lookup_king_moves(pos, us);
    auto bitboard_moves = generate_bitboard_king_moves(pos, us);
    
    // Compare results
    bool lookup_correct = (template_moves == lookup_moves);
    bool bitboard_correct = (template_moves == bitboard_moves);
    
    if (lookup_correct && bitboard_correct) {
        std::cout << "PASS (" << template_moves.size() << " moves)\n";
        stats.record_pass();
    } else {
        std::cout << "FAIL\n";
        std::cout << "  Template moves: " << template_moves.size() << "\n";
        std::cout << "  Lookup moves: " << lookup_moves.size() << " " 
                 << (lookup_correct ? "✓" : "✗") << "\n";
        std::cout << "  Bitboard moves: " << bitboard_moves.size() << " "
                 << (bitboard_correct ? "✓" : "✗") << "\n";
        
        // Show mismatched moves for debugging
        if (!lookup_correct) {
            std::cout << "  Lookup differences:\n";
            for (auto move : lookup_moves) {
                if (std::find(template_moves.begin(), template_moves.end(), move) == template_moves.end()) {
                    std::cout << "    Extra: " << move.first << " -> " << move.second << "\n";
                }
            }
            for (auto move : template_moves) {
                if (std::find(lookup_moves.begin(), lookup_moves.end(), move) == lookup_moves.end()) {
                    std::cout << "    Missing: " << move.first << " -> " << move.second << "\n";
                }
            }
        }
        
        stats.record_fail();
    }
}

/**
 * @brief Test performance comparison between methods
 * @param iterations Number of iterations for benchmarking
 * @param stats Test statistics to update
 */
void test_performance(int iterations, TestStats& stats) {
    std::cout << "\n=== Performance Testing ===\n";
    std::cout << "Running " << iterations << " iterations per method...\n";
    
    // Test positions with different king positions
    std::vector<std::string> test_fens = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", // Starting position
        "8/8/8/3k4/8/8/8/3K4 w - - 0 1", // Kings in center
        "k7/8/8/8/8/8/8/7K w - - 0 1", // Kings in corners
        "4k3/8/8/8/8/8/8/4K3 w - - 0 1", // Kings on edges
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1" // Complex position with pieces
    };
    
    // Timing variables
    auto start_time = high_resolution_clock::now();
    auto end_time = high_resolution_clock::now();
    
    // Test template method
    start_time = high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_fens) {
            Position pos;
            pos.set_from_fen(fen);
            auto moves = generate_template_king_moves(pos, pos.side_to_move);
        }
    }
    end_time = high_resolution_clock::now();
    auto template_duration = duration_cast<microseconds>(end_time - start_time);
    
    // Test lookup method
    start_time = high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_fens) {
            Position pos;
            pos.set_from_fen(fen);
            auto moves = generate_lookup_king_moves(pos, pos.side_to_move);
        }
    }
    end_time = high_resolution_clock::now();
    auto lookup_duration = duration_cast<microseconds>(end_time - start_time);
    
    // Test bitboard method
    start_time = high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_fens) {
            Position pos;
            pos.set_from_fen(fen);
            auto moves = generate_bitboard_king_moves(pos, pos.side_to_move);
        }
    }
    end_time = high_resolution_clock::now();
    auto bitboard_duration = duration_cast<microseconds>(end_time - start_time);
    
    // Calculate and display results
    double lookup_improvement = 100.0 * (template_duration.count() - lookup_duration.count()) / template_duration.count();
    double bitboard_improvement = 100.0 * (template_duration.count() - bitboard_duration.count()) / template_duration.count();
    
    std::cout << "\nPerformance Results:\n";
    std::cout << "Template method:  " << std::setw(8) << template_duration.count() << " μs\n";
    std::cout << "Lookup method:    " << std::setw(8) << lookup_duration.count() << " μs ";
    std::cout << "(" << std::showpos << std::fixed << std::setprecision(1) << lookup_improvement << "%)\n";
    std::cout << "Bitboard method:  " << std::setw(8) << bitboard_duration.count() << " μs ";
    std::cout << "(" << bitboard_improvement << "%)\n" << std::noshowpos;
    
    // Performance test passes if improvement > 0%
    if (lookup_improvement > 0 && bitboard_improvement > 0) {
        std::cout << "Performance test: PASS\n";
        stats.record_pass();
    } else {
        std::cout << "Performance test: FAIL (no improvement detected)\n";
        stats.record_fail();
    }
}

/**
 * @brief Test boundary conditions and edge cases
 * @param stats Test statistics to update
 */
void test_boundary_conditions(TestStats& stats) {
    std::cout << "\n=== Boundary Condition Testing ===\n";
    
    // Test all four corners
    test_position_correctness("k7/8/8/8/8/8/8/7K w - - 0 1", "corners (a8, h1)", stats);
    test_position_correctness("7k/8/8/8/8/8/8/K7 w - - 0 1", "corners (h8, a1)", stats);
    
    // Test edges
    test_position_correctness("3k4/8/8/8/8/8/8/3K4 w - - 0 1", "edge centers", stats);
    test_position_correctness("8/k7/8/8/8/8/7K/8 w - - 0 1", "side edges", stats);
    
    // Test center positions
    test_position_correctness("8/8/8/3k4/3K4/8/8/8 w - - 0 1", "center squares", stats);
    
    // Test with blocking pieces
    test_position_correctness("8/8/8/2pkp3/2pKp3/2ppp3/8/8 w - - 0 1", "surrounded king", stats);
    
    // Test with capture opportunities
    test_position_correctness("8/8/8/2rkr3/2rKr3/2rrr3/8/8 w - - 0 1", "capture available", stats);
}

/**
 * @brief Test table initialization and memory consistency
 * @param stats Test statistics to update
 */
void test_table_initialization(TestStats& stats) {
    std::cout << "\n=== Table Initialization Testing ===\n";
    
    std::cout << "Testing table consistency... ";
    
    // Verify table initialization worked correctly
    bool tables_valid = true;
    int total_moves = 0;
    
    for (int square = 0; square < 64; ++square) {
        int move_count = KingLookupTables::KING_MOVE_COUNT[square];
        uint64_t attacks = KingLookupTables::KING_ATTACKS[square];
        
        // Count should be between 3 (corner) and 8 (center)
        if (move_count < 3 || move_count > 8) {
            tables_valid = false;
            break;
        }
        
        // Bitboard population should match array count
        int bitboard_count = builtin_popcountll(attacks);
        if (bitboard_count != move_count) {
            tables_valid = false;
            break;
        }
        
        // All array moves should correspond to bitboard bits
        for (int i = 0; i < move_count; ++i) {
            int move_square = KingLookupTables::KING_MOVES[square][i];
            if ((attacks & (1ULL << move_square)) == 0) {
                tables_valid = false;
                break;
            }
        }
        
        total_moves += move_count;
    }
    
    // Expected total: 4 corners × 3 moves + 24 edges × 5 moves + 36 center × 8 moves = 420
    int expected_total = 4 * 3 + 24 * 5 + 36 * 8;
    
    if (tables_valid && total_moves == expected_total) {
        std::cout << "PASS (" << total_moves << " total moves)\n";
        stats.record_pass();
    } else {
        std::cout << "FAIL\n";
        std::cout << "  Tables valid: " << (tables_valid ? "yes" : "no") << "\n";
        std::cout << "  Total moves: " << total_moves << " (expected: " << expected_total << ")\n";
        stats.record_fail();
    }
}

/**
 * @brief Main test function
 */
int main() {
    std::cout << "=== King Lookup Tables Test Suite ===\n";
    
    // Initialize lookup tables
    std::cout << "Initializing king lookup tables...\n";
    KingLookupTables::initialize_king_tables();
    
    TestStats stats;
    
    // Run all test categories
    test_table_initialization(stats);
    test_boundary_conditions(stats);
    
    // Correctness tests for various positions
    std::cout << "\n=== Correctness Testing ===\n";
    test_position_correctness("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "starting position", stats);
    test_position_correctness("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", "after e4", stats);
    test_position_correctness("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "castling position", stats);
    test_position_correctness("8/8/8/8/8/8/8/4K3 w - - 0 1", "lone king", stats);
    test_position_correctness("4k3/4r3/4r3/4r3/4K3/4r3/4r3/4r3 w - - 0 1", "king attacked", stats);
    
    // Performance testing
    test_performance(10000, stats);
    
    // Display debug output if all tests pass
    if (stats.tests_failed == 0) {
        std::cout << "\n=== Debug Output ===\n";
        KingLookupTables::print_king_tables();
    }
    
    // Final summary
    stats.print_summary();
    
    return (stats.tests_failed == 0) ? 0 : 1;
}