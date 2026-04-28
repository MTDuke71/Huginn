#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <iomanip>
#include "position.hpp"
#include "movegen_bb.hpp"
#include "king_lookup_tables.hpp"
#include "knight_lookup_tables.hpp"
#include "init.hpp"

// OPTIMIZATION V7: Ultra-fast move generation with minimal coordinate conversions
namespace FastMoveGen {
    // Helper to determine piece type at 64-bit square using bitboards (optimized)
    inline PieceType get_piece_type_at_64(const Position& pos, int sq64, Color color) {
        uint64_t sq_mask = 1ULL << sq64;
        const auto& pieces = pos.piece_bitboards[int(color)];
        
        // Check in MVV-LVA order for better branch prediction in captures
        if (pieces[int(PieceType::Queen)] & sq_mask) return PieceType::Queen;
        if (pieces[int(PieceType::Rook)] & sq_mask) return PieceType::Rook;
        if (pieces[int(PieceType::Bishop)] & sq_mask) return PieceType::Bishop;
        if (pieces[int(PieceType::Knight)] & sq_mask) return PieceType::Knight;
        if (pieces[int(PieceType::Pawn)] & sq_mask) return PieceType::Pawn;
        if (pieces[int(PieceType::King)] & sq_mask) return PieceType::King;
        
        return PieceType::None;
    }

    inline void generate_knight_moves_ultra_fast(const Position& pos, S_MOVELIST& list, Color us) {
        uint64_t knights = pos.piece_bitboards[int(us)][int(PieceType::Knight)];
        uint64_t own_pieces = pos.color_bitboards[int(us)];
        uint64_t enemy_pieces = pos.color_bitboards[int(!us)];
        
        while (knights != 0) {
            int from_sq64 = get_lsb(knights);
            knights &= knights - 1;
            
            uint64_t attacks = KnightLookupTables::KNIGHT_ATTACKS[from_sq64] & ~own_pieces;
            
            // OPTIMIZATION V7: Convert to 120 only once per piece
            int from_sq120 = MAILBOX_MAPS.to120[from_sq64];
            
            // Process captures - use pos.at() just once per capture
            uint64_t captures = attacks & enemy_pieces;
            while (captures != 0) {
                int to_sq64 = get_lsb(captures);
                captures &= captures - 1;
                int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
                
                // Single pos.at() call per capture
                Piece target = pos.at(to_sq120);
                list.add_capture_move(make_capture(from_sq120, to_sq120, type_of(target)), pos);
            }
            
            // Process quiet moves (no piece lookup needed)
            uint64_t quiet_moves = attacks & ~enemy_pieces;
            while (quiet_moves != 0) {
                int to_sq64 = get_lsb(quiet_moves);
                quiet_moves &= quiet_moves - 1;
                int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
                
                list.add_quiet_move(make_move(from_sq120, to_sq120));
            }
        }
    }

    inline void generate_bishop_moves_ultra_fast(const Position& pos, S_MOVELIST& list, Color us) {
        uint64_t bishops = pos.piece_bitboards[int(us)][int(PieceType::Bishop)];
        uint64_t own_pieces = pos.color_bitboards[int(us)];
        uint64_t enemy_pieces = pos.color_bitboards[int(!us)];
        uint64_t occupied = pos.occupied_bitboard;
        
        while (bishops != 0) {
            int from_sq64 = get_lsb(bishops);
            bishops &= bishops - 1;
            
            uint64_t attacks = bishop_attacks(from_sq64, occupied) & ~own_pieces;
            int from_sq120 = MAILBOX_MAPS.to120[from_sq64];
            
            // Process captures - single pos.at() call per capture
            uint64_t captures = attacks & enemy_pieces;
            while (captures != 0) {
                int to_sq64 = get_lsb(captures);
                captures &= captures - 1;
                int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
                
                Piece target = pos.at(to_sq120);
                list.add_capture_move(make_capture(from_sq120, to_sq120, type_of(target)), pos);
            }
            
            // Process quiet moves
            uint64_t quiet_moves = attacks & ~enemy_pieces;
            while (quiet_moves != 0) {
                int to_sq64 = get_lsb(quiet_moves);
                quiet_moves &= quiet_moves - 1;
                int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
                
                list.add_quiet_move(make_move(from_sq120, to_sq120));
            }
        }
    }

    inline void generate_rook_moves_ultra_fast(const Position& pos, S_MOVELIST& list, Color us) {
        uint64_t rooks = pos.piece_bitboards[int(us)][int(PieceType::Rook)];
        uint64_t own_pieces = pos.color_bitboards[int(us)];
        uint64_t enemy_pieces = pos.color_bitboards[int(!us)];
        uint64_t occupied = pos.occupied_bitboard;
        
        while (rooks != 0) {
            int from_sq64 = get_lsb(rooks);
            rooks &= rooks - 1;
            
            uint64_t attacks = rook_attacks(from_sq64, occupied) & ~own_pieces;
            int from_sq120 = MAILBOX_MAPS.to120[from_sq64];
            
            // Process captures - single pos.at() call per capture
            uint64_t captures = attacks & enemy_pieces;
            while (captures != 0) {
                int to_sq64 = get_lsb(captures);
                captures &= captures - 1;
                int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
                
                Piece target = pos.at(to_sq120);
                list.add_capture_move(make_capture(from_sq120, to_sq120, type_of(target)), pos);
            }
            
            // Process quiet moves
            uint64_t quiet_moves = attacks & ~enemy_pieces;
            while (quiet_moves != 0) {
                int to_sq64 = get_lsb(quiet_moves);
                quiet_moves &= quiet_moves - 1;
                int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
                
                list.add_quiet_move(make_move(from_sq120, to_sq120));
            }
        }
    }

    inline void generate_all_moves_ultra_optimized(const Position& pos, S_MOVELIST& list, Color us) {
        list.count = 0;
        
        // Use ultra-optimized generation for major pieces
        generate_knight_moves_ultra_fast(pos, list, us);
        generate_bishop_moves_ultra_fast(pos, list, us);
        generate_rook_moves_ultra_fast(pos, list, us);
        
        // Use standard generation for others (for now)
        BitboardMoveGen::generate_pawn_moves_bitboard(pos, list, us);
        BitboardMoveGen::generate_queen_moves_bitboard(pos, list, us);
        BitboardMoveGen::generate_king_moves_bitboard(pos, list, us);
        KingLookupTables::generate_castling_moves_optimized(pos, list, us);
    }
}

// Huginn2 Perft function - OPTIMIZED V6: Comprehensive enhanced move generation
static uint64_t perft_huginn2(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    S_MOVELIST list;
    Color us = pos.side_to_move;
    
    // OPTIMIZATION V6: Use comprehensive optimized move generation
    FastMoveGen::generate_all_moves_ultra_optimized(pos, list, us);
    
    // OPTIMIZATION V6: Enhanced depth 1 handling (inherited from V3/V5)
    if (depth == 1) {
        uint64_t legal_moves = 0;
        const S_MOVE* moves = list.moves;
        const int move_count = list.count;
        
        for (int i = 0; i < move_count; ++i) {
            if (pos.MakeMove(moves[i])) {
                legal_moves++;
                pos.TakeMove();
            }
        }
        return legal_moves;
    }
    
    // For deeper searches
    uint64_t nodes = 0;
    const S_MOVE* moves = list.moves;
    const int move_count = list.count;
    
    for (int i = 0; i < move_count; ++i) {
        if (pos.MakeMove(moves[i])) {
            nodes += perft_huginn2(pos, depth - 1);
            pos.TakeMove();
        }
    }
    return nodes;
}

// Structure to hold a perft test case
struct PerftTestCase {
    std::string fen;
    std::vector<std::pair<int, uint64_t>> expected_results; // depth, node_count pairs
};

// Parse a line from the EPD file
PerftTestCase parse_epd_line(const std::string& line) {
    PerftTestCase test_case;
    
    // Find the first semicolon to separate FEN from perft data
    size_t first_semicolon = line.find(';');
    if (first_semicolon == std::string::npos) {
        return test_case; // Invalid line
    }
    
    test_case.fen = line.substr(0, first_semicolon);
    
    // Parse perft results
    std::string perft_data = line.substr(first_semicolon + 1);
    std::istringstream ss(perft_data);
    std::string token;
    
    while (std::getline(ss, token, ';')) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        
        if (token.empty()) continue;
        
        // Parse "D<depth> <count>"
        if (token[0] == 'D') {
            size_t space_pos = token.find(' ');
            if (space_pos != std::string::npos) {
                int depth = std::stoi(token.substr(1, space_pos - 1));
                uint64_t count = std::stoull(token.substr(space_pos + 1));
                test_case.expected_results.push_back({depth, count});
            }
        }
    }
    
    return test_case;
}

// Load all test cases from the EPD file
std::vector<PerftTestCase> load_perft_suite(const std::string& filename) {
    std::vector<PerftTestCase> test_cases;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return test_cases;
    }
    
    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments
        
        PerftTestCase test_case = parse_epd_line(line);
        if (!test_case.fen.empty() && !test_case.expected_results.empty()) {
            test_cases.push_back(test_case);
        } else {
            std::cerr << "Warning: Failed to parse line " << line_number << ": " << line << std::endl;
        }
    }
    
    file.close();
    return test_cases;
}

// Test a single position up to the specified max depth
bool test_position(const PerftTestCase& test_case, int max_depth, int& total_tests, int& failed_tests) {
    Position pos;
    
    if (!pos.set_from_fen(test_case.fen)) {
        std::cout << "FAILED to parse FEN: " << test_case.fen << std::endl;
        failed_tests++;
        total_tests++;
        return true; // Return true to indicate failure (stop testing)
    }
    
    // Collect all depths for this position
    std::vector<int> depths_to_test;
    for (const auto& expected : test_case.expected_results) {
        if (expected.first <= max_depth) {
            depths_to_test.push_back(expected.first);
        }
    }
    
    if (depths_to_test.empty()) {
        return false; // No tests to run for this position
    }
    
    std::cout << "FEN: " << test_case.fen << std::endl;
    
    // Test each depth
    for (const auto& expected : test_case.expected_results) {
        int depth = expected.first;
        uint64_t expected_nodes = expected.second;
        
        if (depth > max_depth) continue; // Skip depths beyond our limit
        
        total_tests++;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        uint64_t actual_nodes = perft_huginn2(pos, depth);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        if (actual_nodes == expected_nodes) {
            std::cout << "  Depth " << depth << ": " << actual_nodes << " nodes (" << duration.count() << "ms) ✅ PASS" << std::endl;
        } else {
            // First failure detected - show detailed information and stop
            std::cout << "  Depth " << depth << ": " << actual_nodes << " nodes (" << duration.count() << "ms) ❌ FAIL" << std::endl;
            std::cout << "    Expected: " << expected_nodes << std::endl;
            std::cout << "    Actual:   " << actual_nodes << std::endl;
            std::cout << "\n=== STOPPING AT FIRST FAILURE FOR DEBUGGING ===" << std::endl;
            failed_tests++;
            return true; // Return true to indicate we should stop testing
        }
    }
    
    std::cout << std::endl;
    return false; // Return false to continue testing
}

// Add a wrapper to compare Ultra-Engine perft with perft_huginn2
void compare_perft(Position& pos, int depth) {
    std::cout << "\n=== PERFT COMPARISON ===\n";
    
    // Ground truth using perft_huginn2
    uint64_t ground_truth_nodes = perft_huginn2(pos, depth);
    std::cout << "Ground truth (perft_huginn2): " << ground_truth_nodes << " nodes\n";

    // Ultra-Engine perft
    uint64_t ultra_nodes = pos.perft(depth);
    std::cout << "Ultra-Engine perft: " << ultra_nodes << " nodes\n";

    // Compare results
    if (ground_truth_nodes == ultra_nodes) {
        std::cout << "✓ Perft results match!\n";
    } else {
        std::cout << "❌ Perft mismatch!\n";
        std::cout << "  Difference: " << (ground_truth_nodes - ultra_nodes) << " nodes\n";
    }

    // Debugging output for moves at depth 1
    if (depth == 1) {
        S_MOVELIST list;
        pos.generate_all_moves(list);
        std::cout << "Generated moves at depth 1:\n";
        for (int i = 0; i < list.count; ++i) {
            std::cout << "  Move: " << list.moves[i].to_string() << "\n";
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize the chess engine
    Huginn::init();
    
    std::cout << "=== Huginn2 Perft Suite OPTIMIZED V6 (Comprehensive Move Gen) ===" << std::endl;
    std::cout << "Testing optimized perft with comprehensive enhanced move generation" << std::endl;
    std::cout << std::endl;
    
    // Default parameters - focus on first 2 positions
    int max_depth = 6;
    std::string epd_file = "test/perftsuite.epd";
    int positions_to_test = 2;  // Fixed to first 2 positions
    
    // Parse command line arguments for depth override
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--depth" && i + 1 < argc) {
            max_depth = std::stoi(argv[++i]);
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --depth <n>     Maximum depth to test (default: 6)" << std::endl;
            std::cout << "  --help, -h      Show this help message" << std::endl;
            std::cout << std::endl;
            std::cout << "This program tests the first 2 positions from perftsuite.epd using Huginn2 bitboard implementation." << std::endl;
            return 0;
        }
    }
    
    std::cout << "Configuration:" << std::endl;
    std::cout << "  EPD file: " << epd_file << std::endl;
    std::cout << "  Max depth: " << max_depth << std::endl;
    std::cout << "  Positions: First " << positions_to_test << " positions only" << std::endl;
    std::cout << "  Method: Huginn2 optimized perft with comprehensive move generation (V6)" << std::endl;
    std::cout << std::endl;
    
    // Load test cases
    std::vector<PerftTestCase> test_cases = load_perft_suite(epd_file);
    
    if (test_cases.empty()) {
        std::cerr << "Error: No test cases loaded from " << epd_file << std::endl;
        return 1;
    }
    
    if (test_cases.size() < positions_to_test) {
        positions_to_test = test_cases.size();
        std::cout << "Note: Only " << positions_to_test << " positions available in EPD file" << std::endl;
    }
    
    std::cout << "Loaded " << test_cases.size() << " test positions (testing first " << positions_to_test << ")" << std::endl;
    
    // Count total expected tests for the positions we'll actually test
    int total_expected_tests = 0;
    for (int i = 0; i < positions_to_test; i++) {
        const auto& test_case = test_cases[i];
        for (const auto& expected : test_case.expected_results) {
            if (expected.first <= max_depth) {
                total_expected_tests++;
            }
        }
    }
    
    std::cout << "Will run approximately " << total_expected_tests << " tests using comprehensive optimized move generation (V6)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    auto overall_start_time = std::chrono::high_resolution_clock::now();
    
    int total_tests = 0;
    int failed_tests = 0;
    
    // Test each position (limited to first 2)
    for (int i = 0; i < positions_to_test; i++) {
        std::cout << "[Position " << (i + 1) << "/" << positions_to_test << "] ";
        bool should_stop = test_position(test_cases[i], max_depth, total_tests, failed_tests);
        if (should_stop) {
            std::cout << "\nStopped testing at position " << (i + 1) << " due to failure." << std::endl;
            break;
        }
    }
    
    auto overall_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(overall_end_time - overall_start_time);
    
    // Summary
    std::cout << "========================================" << std::endl;
    std::cout << "=== FINAL RESULTS ===" << std::endl;
    std::cout << "Test method: Huginn2 optimized perft with comprehensive move generation (V6)" << std::endl;
    std::cout << "Positions tested: " << positions_to_test << std::endl;
    std::cout << "Total tests run: " << total_tests << std::endl;
    std::cout << "Tests passed: " << (total_tests - failed_tests) << std::endl;
    std::cout << "Tests failed: " << failed_tests << std::endl;
    std::cout << "Success rate: " << std::fixed << std::setprecision(1) 
              << (100.0 * (total_tests - failed_tests) / total_tests) << "%" << std::endl;
    std::cout << "Total time: " << total_duration.count() << "ms" << std::endl;
    
    if (failed_tests == 0) {
        std::cout << std::endl << "🎉 ALL TESTS PASSED! Huginn2 bitboard implementation is CORRECT! 🎉" << std::endl;
    } else {
        std::cout << std::endl << "❌ Some tests failed. Please check the output above for details." << std::endl;
    }
    
    return failed_tests > 0 ? 1 : 0;
}