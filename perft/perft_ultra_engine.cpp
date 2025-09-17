/**
 * @file perft_ultra_engine.cpp
 * @brief TRUE UltraEngine perft implementation using pure 64-bit coordinate system
 * 
 * This is the actual UltraEngine perft that uses:
 * - UltraPosition instead of Position
 * - UltraMoveList instead of S_MOVELIST  
 * - UltraMove instead of S_MOVE
 * - Pure 64-bit coordinates (no conversions)
 * 
 * Expected performance improvement: 30-50% faster than legacy system
 * 
 * @author MTDuke71
 * @version Ultra 1.0
 */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_move.hpp" 
#include "../ultra_engine/ultra_attacks.hpp"
#include "../ultra_engine/ultra_knight_gen.hpp"
#include "../ultra_engine/ultra_sliding_gen.hpp"
#include "../ultra_engine/ultra_pawn_gen.hpp"
#include "../src/epd_parser.hpp"

using namespace UltraEngine;

// ============================================================================
// TRUE ULTRA ENGINE PERFT - Pure 64-bit coordinate system
// ============================================================================

/**
 * @brief Ultra-fast perft using pure UltraEngine system
 * @param pos UltraPosition with 64-bit coordinates
 * @param depth Search depth
 * @return Node count
 */
static uint64_t perft_ultra_engine(UltraPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    // Ultra-fast depth 1 handling
    if (depth == 1) {
        uint64_t legal_moves = 0;
        for (int i = 0; i < moves.size(); ++i) {
            if (pos.is_legal_move(moves[i])) {
                legal_moves++;
            }
        }
        return legal_moves;
    }
    
    // Deeper search
    uint64_t nodes = 0;
    for (int i = 0; i < moves.size(); ++i) {
        if (pos.is_legal_move(moves[i])) {
            pos.make_move(moves[i]);
            nodes += perft_ultra_engine(pos, depth - 1);
            pos.unmake_move(moves[i]);
        }
    }
    return nodes;
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Simple trim function
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

// Parse single EPD line into TestPosition format
TestPosition parse_epd_line(const std::string& line) {
    TestPosition result;
    std::istringstream iss(line);
    std::string token;
    
    // Extract FEN (first 4 tokens: position, side, castling, en passant)
    std::vector<std::string> fen_parts;
    for (int i = 0; i < 4 && iss >> token; ++i) {
        fen_parts.push_back(token);
    }
    
    if (fen_parts.size() >= 4) {
        result.fen = fen_parts[0] + " " + fen_parts[1] + " " + fen_parts[2] + " " + fen_parts[3] + " 0 1";
        
        // Parse the rest for perft results
        std::string remainder;
        std::getline(iss, remainder);
        
        // Look for D1, D2, etc.
        size_t pos = 0;
        while ((pos = remainder.find('D', pos)) != std::string::npos) {
            if (pos > 0 && std::isalnum(remainder[pos-1])) {
                pos++;
                continue;
            }
            
            size_t depth_start = pos + 1;
            size_t depth_end = remainder.find_first_not_of("0123456789", depth_start);
            if (depth_end == std::string::npos) depth_end = remainder.length();
            
            if (depth_end > depth_start) {
                int depth = std::stoi(remainder.substr(depth_start, depth_end - depth_start));
                
                size_t value_start = remainder.find_first_of("0123456789", depth_end);
                if (value_start != std::string::npos) {
                    size_t value_end = remainder.find_first_not_of("0123456789", value_start);
                    if (value_end == std::string::npos) value_end = remainder.length();
                    
                    uint64_t nodes = std::stoull(remainder.substr(value_start, value_end - value_start));
                    result.perft_results.emplace_back(depth, nodes);
                }
            }
            pos = depth_end;
        }
    }
    
    return result;
}

// ============================================================================
// PERFT TEST INFRASTRUCTURE
// ============================================================================

struct PerftTestCase {
    std::string fen;
    std::vector<std::pair<int, uint64_t>> expected_results; // depth, node_count pairs
};

// Load test cases from EPD file using the new parsing
std::vector<PerftTestCase> load_perft_tests(const std::string& filename, int max_positions = -1) {
    std::vector<PerftTestCase> tests;
    auto epd_tests = parse_epd_file(filename);
    
    for (size_t i = 0; i < epd_tests.size() && (max_positions == -1 || (int)i < max_positions); ++i) {
        PerftTestCase test;
        test.fen = epd_tests[i].fen;
        test.expected_results = epd_tests[i].perft_results;
        tests.push_back(test);
    }
    
    std::cout << "Loaded " << tests.size() << " test positions";
    if (max_positions > 0) {
        std::cout << " (testing first " << max_positions << ")";
    }
    std::cout << std::endl;
    
    return tests;
}

// ============================================================================
// PERFORMANCE TESTING FRAMEWORK
// ============================================================================

void run_ultra_engine_perft_suite() {
    std::cout << "=== TRUE UltraEngine Perft Suite ===\n";
    std::cout << "Testing pure 64-bit coordinate system with zero conversions\n\n";
    
    const std::string epd_file = "test/perftsuite.epd";
    const int max_depth = 6;
    const int max_positions = 2; // Start with 2 positions for comparison
    
    std::cout << "Configuration:\n";
    std::cout << "  EPD file: " << epd_file << "\n";
    std::cout << "  Max depth: " << max_depth << "\n";
    std::cout << "  Positions: First " << max_positions << " positions only\n";
    std::cout << "  Method: TRUE UltraEngine with pure 64-bit coordinates\n\n";
    
    auto test_cases = load_perft_tests(epd_file, max_positions);
    if (test_cases.empty()) {
        std::cerr << "No test cases loaded. Exiting.\n";
        return;
    }
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Count total tests
    for (const auto& test : test_cases) {
        for (const auto& result : test.expected_results) {
            if (result.first <= max_depth) {
                total_tests++;
            }
        }
    }
    
    std::cout << "Will run approximately " << total_tests << " tests using TRUE UltraEngine system\n";
    std::cout << "========================================\n\n";
    
    for (size_t i = 0; i < test_cases.size(); ++i) {
        const auto& test = test_cases[i];
        
        std::cout << "[Position " << (i + 1) << "/" << test_cases.size() << "] FEN: " << test.fen << "\n";
        
        // Create UltraPosition from FEN
        UltraPosition pos;
        if (!pos.set_fen(test.fen)) {
            std::cout << "  ERROR: Failed to set FEN\n";
            continue;
        }
        
        for (const auto& expected : test.expected_results) {
            int depth = expected.first;
            uint64_t expected_nodes = expected.second;
            
            if (depth > max_depth) continue;
            
            auto test_start = std::chrono::high_resolution_clock::now();
            uint64_t nodes = perft_ultra_engine(pos, depth);
            auto test_end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(test_end - test_start);
            
            bool passed = (nodes == expected_nodes);
            std::string status = passed ? "✓ PASS" : "✗ FAIL";
            
            std::cout << "  Depth " << depth << ": " << nodes << " nodes (" 
                      << duration.count() << "ms) " << status;
            
            if (!passed) {
                std::cout << " (expected " << expected_nodes << ")";
                failed_tests++;
            } else {
                passed_tests++;
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "========================================\n";
    std::cout << "=== FINAL RESULTS ===\n";
    std::cout << "Test method: TRUE UltraEngine with pure 64-bit coordinates\n";
    std::cout << "Positions tested: " << test_cases.size() << "\n";
    std::cout << "Total tests run: " << (passed_tests + failed_tests) << "\n";
    std::cout << "Tests passed: " << passed_tests << "\n";
    std::cout << "Tests failed: " << failed_tests << "\n";
    std::cout << "Success rate: " << std::fixed << std::setprecision(1) 
              << (100.0 * passed_tests / (passed_tests + failed_tests)) << "%\n";
    std::cout << "Total time: " << total_duration.count() << "ms\n\n";
    
    if (failed_tests == 0) {
        std::cout << "🚀 ALL TESTS PASSED! TRUE UltraEngine is CORRECT and BLAZING FAST! 🚀\n";
    } else {
        std::cout << "❌ " << failed_tests << " test(s) failed. UltraEngine needs debugging.\n";
    }
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    try {
        // Initialize UltraEngine attack tables
        UltraEngine::UltraAttacks::initialize();
        std::cout << "UltraEngine attack tables initialized successfully.\n";
        
        // Run the TRUE UltraEngine perft suite
        run_ultra_engine_perft_suite();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}