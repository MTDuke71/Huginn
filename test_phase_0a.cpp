/**
 * @file test_phase_0a.cpp
 * @brief Test program for Phase 0A bitboard infrastructure
 * 
 * Validates the newly implemented attack generation functions and benchmarking
 * framework. Ensures correctness before proceeding with the bitboard migration.
 * 
 * Tests Include:
 * - Pawn attack bitboard generation
 * - Basic sliding piece attacks (bishop/rook)  
 * - Bitboard iteration utilities
 * - Performance benchmarking framework
 * 
 * @author MTDuke71
 * @version 1.0
 * @see PHASE_0_ASSESSMENT.md for infrastructure overview
 */

#include "position.hpp"
#include "bitboard.hpp"
#include "bitboard_benchmark.hpp" 
#include "pawn_lookup_tables.hpp"
#include "knight_lookup_tables.hpp"
#include "king_lookup_tables.hpp"
#include "parse_fen.hpp"
#include <iostream>
#include <cassert>

/**
 * Test pawn attack bitboard generation
 */
void test_pawn_attacks() {
    std::cout << "Testing pawn attack bitboards...\n";
    
    // Test white pawn attacks from e4 (square 28 in 64-square notation)
    uint64_t white_attacks = PawnLookupTables::get_pawn_attacks(Color::White, 28);
    
    // White pawn on e4 should attack d5 (35) and f5 (37)
    bool attacks_d5 = (white_attacks & (1ULL << 35)) != 0;
    bool attacks_f5 = (white_attacks & (1ULL << 37)) != 0;
    
    std::cout << "  White pawn on e4 attacks d5: " << (attacks_d5 ? "YES" : "NO") << "\n";
    std::cout << "  White pawn on e4 attacks f5: " << (attacks_f5 ? "YES" : "NO") << "\n";
    
    // Test black pawn attacks from e5 (square 36)
    uint64_t black_attacks = PawnLookupTables::get_pawn_attacks(Color::Black, 36);
    
    // Black pawn on e5 should attack d4 (27) and f4 (29)
    bool attacks_d4 = (black_attacks & (1ULL << 27)) != 0;
    bool attacks_f4 = (black_attacks & (1ULL << 29)) != 0;
    
    std::cout << "  Black pawn on e5 attacks d4: " << (attacks_d4 ? "YES" : "NO") << "\n";
    std::cout << "  Black pawn on e5 attacks f4: " << (attacks_f4 ? "YES" : "NO") << "\n";
    
    assert(attacks_d5 && attacks_f5 && attacks_d4 && attacks_f4);
    std::cout << "  ✓ Pawn attack tests passed!\n\n";
}

/**
 * Test sliding piece attack generation
 */
void test_sliding_attacks() {
    std::cout << "Testing sliding piece attacks...\n";
    
    // Test bishop attacks from d4 (27) with empty board
    uint64_t bishop_attacks_empty = bishop_attacks(27, 0ULL);
    int bishop_attack_count = popcount(bishop_attacks_empty);
    
    std::cout << "  Bishop on d4 (empty board) attacks " << bishop_attack_count << " squares\n";
    
    // Test rook attacks from d4 (27) with empty board  
    uint64_t rook_attacks_empty = rook_attacks(27, 0ULL);
    int rook_attack_count = popcount(rook_attacks_empty);
    
    std::cout << "  Rook on d4 (empty board) attacks " << rook_attack_count << " squares\n";
    
    // Test with some occupancy
    uint64_t occupancy = (1ULL << 35) | (1ULL << 19);  // Pieces on d5 and d3
    uint64_t rook_attacks_blocked = rook_attacks(27, occupancy);
    int blocked_attack_count = popcount(rook_attacks_blocked);
    
    std::cout << "  Rook on d4 (with blocking pieces) attacks " << blocked_attack_count << " squares\n";
    
    // Basic sanity checks
    assert(bishop_attack_count > 0 && bishop_attack_count <= 13);  // Maximum 13 diagonal squares
    assert(rook_attack_count > 0 && rook_attack_count <= 14);      // Maximum 14 orthogonal squares
    assert(blocked_attack_count <= rook_attack_count);             // Blocking should reduce or equal attacks
    
    std::cout << "  ✓ Sliding piece attack tests passed!\n\n";
}

/**
 * Test bitboard iteration utilities
 */
void test_bitboard_iteration() {
    std::cout << "Testing bitboard iteration utilities...\n";
    
    // Create a test bitboard with a few bits set
    uint64_t test_bb = (1ULL << 10) | (1ULL << 25) | (1ULL << 50);
    
    std::cout << "  Test bitboard has " << popcount(test_bb) << " bits set\n";
    
    // Test LSB extraction
    int first_bit = peek_lsb(test_bb);
    std::cout << "  First bit (LSB): " << first_bit << "\n";
    
    // Test iteration with while loop
    uint64_t temp_bb = test_bb;
    int count = 0;
    while (temp_bb != 0) {
        int square = get_lsb(temp_bb);
        temp_bb &= temp_bb - 1;  // Remove LSB
        count++;
        std::cout << "    Extracted square: " << square << "\n";
    }
    
    assert(count == 3);  // Should extract exactly 3 squares
    assert(first_bit == 10);  // LSB should be square 10
    
    std::cout << "  ✓ Bitboard iteration tests passed!\n\n";
}

/**
 * Test performance benchmarking framework
 */
void test_benchmarking_framework() {
    std::cout << "Testing benchmarking framework...\n";
    
    // Create a test position
    Position pos;
    parse_fen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Run small benchmark tests
    auto knight_results = BitboardBenchmark::benchmark_knight_attacks(pos, 1000);
    auto pawn_results = BitboardBenchmark::benchmark_pawn_attacks(pos, 1000);
    
    std::cout << "  Knight benchmark completed: " << knight_results.iterations << " iterations\n";
    std::cout << "  Pawn benchmark completed: " << pawn_results.iterations << " iterations\n";
    
    // Verify results are reasonable
    assert(knight_results.piece_list_time_ns >= 0);
    assert(knight_results.bitboard_time_ns >= 0);
    assert(knight_results.speedup_factor > 0);
    assert(pawn_results.piece_list_time_ns >= 0);
    assert(pawn_results.bitboard_time_ns >= 0);
    assert(pawn_results.speedup_factor > 0);
    
    std::cout << "  ✓ Benchmarking framework tests passed!\n\n";
}

int main() {
    std::cout << "=== Phase 0A Bitboard Infrastructure Test ===\n\n";
    
    try {
        // Initialize lookup tables
        KnightLookupTables::initialize_knight_tables();
        KingLookupTables::initialize_king_tables();
        PawnLookupTables::initialize_pawn_tables();
        
        // Run all tests
        test_pawn_attacks();
        test_sliding_attacks();
        test_bitboard_iteration();
        test_benchmarking_framework();
        
        std::cout << "=== All Phase 0A Tests Passed! ===\n";
        std::cout << "✓ Pawn attack bitboards working\n";
        std::cout << "✓ Sliding piece attacks implemented\n";
        std::cout << "✓ Bitboard iteration utilities functional\n";
        std::cout << "✓ Performance benchmarking framework ready\n";
        std::cout << "\nPhase 0A infrastructure is complete and validated.\n";
        std::cout << "Ready to proceed with Phase 1: Attack Detection Migration.\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}