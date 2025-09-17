/**
 * @file test_ultra_attacks.cpp
 * @brief Test suite for ultra-fast attack table system
 */

#include "ultra_attacks.hpp"
#include <iostream>
#include <chrono>
#include <cassert>

using namespace UltraEngine;

// ============================================================================
// CORRECTNESS TESTS
// ============================================================================

void test_initialization() {
    std::cout << "Testing attack table initialization...\n";
    
    assert(!UltraAttacks::is_initialized());
    
    UltraAttacks::initialize();
    assert(UltraAttacks::is_initialized());
    
    std::cout << "✅ Attack table initialization test passed!\n";
}

void test_knight_attacks() {
    std::cout << "Testing knight attacks...\n";
    
    // Test knight on e4 (square 28)
    uint64_t attacks = UltraAttacks::knight(28);
    
    // Knight on e4 should attack: d2, f2, c3, g3, c5, g5, d6, f6
    // Squares: 11, 13, 18, 22, 34, 38, 43, 45
    uint64_t expected = 0ULL;
    expected |= 1ULL << 11; // d2
    expected |= 1ULL << 13; // f2
    expected |= 1ULL << 18; // c3
    expected |= 1ULL << 22; // g3
    expected |= 1ULL << 34; // c5
    expected |= 1ULL << 38; // g5
    expected |= 1ULL << 43; // d6
    expected |= 1ULL << 45; // f6
    
    assert(attacks == expected);
    
    // Test knight on a1 (corner case)
    attacks = UltraAttacks::knight(0);
    expected = (1ULL << 10) | (1ULL << 17); // c2, b3
    assert(attacks == expected);
    
    std::cout << "✅ Knight attacks test passed!\n";
}

void test_king_attacks() {
    std::cout << "Testing king attacks...\n";
    
    // Test king on e4 (square 28)
    uint64_t attacks = UltraAttacks::king(28);
    
    // King on e4 should attack all 8 surrounding squares
    uint64_t expected = 0ULL;
    expected |= 1ULL << 19; // d3
    expected |= 1ULL << 20; // e3
    expected |= 1ULL << 21; // f3
    expected |= 1ULL << 27; // d4
    expected |= 1ULL << 29; // f4
    expected |= 1ULL << 35; // d5
    expected |= 1ULL << 36; // e5
    expected |= 1ULL << 37; // f5
    
    assert(attacks == expected);
    
    // Test king on a1 (corner case)
    attacks = UltraAttacks::king(0);
    expected = (1ULL << 1) | (1ULL << 8) | (1ULL << 9); // b1, a2, b2
    assert(attacks == expected);
    
    std::cout << "✅ King attacks test passed!\n";
}

void test_pawn_attacks() {
    std::cout << "Testing pawn attacks...\n";
    
    // Test white pawn on e4 (square 28)
    uint64_t attacks = UltraAttacks::pawn(28, 0); // White
    uint64_t expected = (1ULL << 35) | (1ULL << 37); // d5, f5
    assert(attacks == expected);
    
    // Test black pawn on e5 (square 36)
    attacks = UltraAttacks::pawn(36, 1); // Black
    expected = (1ULL << 27) | (1ULL << 29); // d4, f4
    assert(attacks == expected);
    
    // Test pawn on a-file (edge case)
    attacks = UltraAttacks::pawn(8, 0); // White pawn on a2
    expected = 1ULL << 17; // b3 only
    assert(attacks == expected);
    
    std::cout << "✅ Pawn attacks test passed!\n";
}

void test_rook_attacks() {
    std::cout << "Testing rook attacks...\n";
    
    // Test rook on e4 with no blockers
    uint64_t empty_board = 0ULL;
    uint64_t attacks = UltraAttacks::rook(28, empty_board);
    
    // Should attack entire rank 4 and file e (excluding e4 itself)
    uint64_t expected = 0ULL;
    // Rank 4: a4, b4, c4, d4, f4, g4, h4
    for (int f = 0; f < 8; ++f) {
        if (f != 4) expected |= 1ULL << (24 + f); // rank 4, different files
    }
    // File e: e1, e2, e3, e5, e6, e7, e8
    for (int r = 0; r < 8; ++r) {
        if (r != 4) expected |= 1ULL << (r * 8 + 4); // different ranks, file e
    }
    
    assert(attacks == expected);
    
    // Test with blockers
    uint64_t blockers = (1ULL << 26) | (1ULL << 30); // c4, g4
    attacks = UltraAttacks::rook(28, blockers);
    
    // Should stop at blockers
    expected = 0ULL;
    expected |= 1ULL << 24; // a4
    expected |= 1ULL << 25; // b4
    expected |= 1ULL << 26; // c4 (blocker included)
    expected |= 1ULL << 27; // d4
    expected |= 1ULL << 29; // f4
    expected |= 1ULL << 30; // g4 (blocker included)
    // Vertical moves still unlimited
    for (int r = 0; r < 8; ++r) {
        if (r != 4) expected |= 1ULL << (r * 8 + 4);
    }
    
    assert(attacks == expected);
    
    std::cout << "✅ Rook attacks test passed!\n";
}

void test_bishop_attacks() {
    std::cout << "Testing bishop attacks...\n";
    
    // Test bishop on e4 with no blockers
    uint64_t empty_board = 0ULL;
    uint64_t attacks = UltraAttacks::bishop(28, empty_board);
    
    // Should attack all diagonal squares
    uint64_t expected = 0ULL;
    // Main diagonal (a8-h1): f3, g2, h1, d5, c6, b7, a8
    expected |= 1ULL << 21; // f3
    expected |= 1ULL << 14; // g2
    expected |= 1ULL << 7;  // h1
    expected |= 1ULL << 35; // d5
    expected |= 1ULL << 42; // c6
    expected |= 1ULL << 49; // b7
    expected |= 1ULL << 56; // a8
    
    // Anti-diagonal (h8-a1): d3, c2, b1, f5, g6, h7
    expected |= 1ULL << 19; // d3
    expected |= 1ULL << 10; // c2
    expected |= 1ULL << 1;  // b1
    expected |= 1ULL << 37; // f5
    expected |= 1ULL << 46; // g6
    expected |= 1ULL << 55; // h7
    
    assert(attacks == expected);
    
    std::cout << "✅ Bishop attacks test passed!\n";
}

void test_queen_attacks() {
    std::cout << "Testing queen attacks...\n";
    
    // Queen attacks should be combination of rook and bishop
    uint64_t empty_board = 0ULL;
    uint64_t queen_attacks = UltraAttacks::queen(28, empty_board);
    uint64_t rook_attacks = UltraAttacks::rook(28, empty_board);
    uint64_t bishop_attacks = UltraAttacks::bishop(28, empty_board);
    
    assert(queen_attacks == (rook_attacks | bishop_attacks));
    
    std::cout << "✅ Queen attacks test passed!\n";
}

// ============================================================================
// PERFORMANCE TESTS
// ============================================================================

void test_attack_performance() {
    std::cout << "Testing attack lookup performance...\n";
    
    const int NUM_LOOKUPS = 10000000;
    const uint64_t occupied = 0x123456789ABCDEFULL; // Random occupied squares
    
    // Test knight attack performance
    auto start = std::chrono::high_resolution_clock::now();
    volatile uint64_t checksum = 0;
    for (int i = 0; i < NUM_LOOKUPS; ++i) {
        checksum += UltraAttacks::knight(i & 63);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Knight lookups: " << NUM_LOOKUPS << " in " << duration.count() 
              << " µs (" << (NUM_LOOKUPS * 1000000LL / duration.count()) << " lookups/sec)\n";
    
    // Test rook attack performance (magic bitboards)
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_LOOKUPS; ++i) {
        checksum += UltraAttacks::rook(i & 63, occupied);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Rook lookups: " << NUM_LOOKUPS << " in " << duration.count() 
              << " µs (" << (NUM_LOOKUPS * 1000000LL / duration.count()) << " lookups/sec)\n";
    
    // Test bishop attack performance
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_LOOKUPS; ++i) {
        checksum += UltraAttacks::bishop(i & 63, occupied);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Bishop lookups: " << NUM_LOOKUPS << " in " << duration.count() 
              << " µs (" << (NUM_LOOKUPS * 1000000LL / duration.count()) << " lookups/sec)\n";
    
    std::cout << "Checksum: " << checksum << " (prevents optimization)\n";
    std::cout << "✅ Attack performance test completed!\n";
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main() {
    std::cout << "=== UltraAttacks System Test Suite ===\n\n";
    
    try {
        // Initialize attack tables
        test_initialization();
        
        std::cout << "\n";
        
        // Correctness tests
        test_knight_attacks();
        test_king_attacks();
        test_pawn_attacks();
        test_rook_attacks();
        test_bishop_attacks();
        test_queen_attacks();
        
        std::cout << "\n";
        
        // Performance tests
        test_attack_performance();
        
        std::cout << "\n🎉 All UltraAttacks tests passed! System is clean and ready.\n";
        
        // Display key benefits
        std::cout << "\n=== UltraAttacks System Benefits ===\n";
        std::cout << "✅ Pure 64-bit coordinates - zero conversion overhead\n";
        std::cout << "✅ Magic bitboard sliding piece attacks - ultra-fast\n";
        std::cout << "✅ Pre-computed lookup tables for leaping pieces\n";
        std::cout << "✅ Optimized memory layout for cache efficiency\n";
        std::cout << "✅ Advanced attack queries for legal move generation\n";
        std::cout << "✅ Ready for integration with ultra-fast move generation\n";
        
        // Cleanup
        UltraAttacks::cleanup();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Test failed with exception: " << e.what() << "\n";
        UltraAttacks::cleanup();
        return 1;
    } catch (...) {
        std::cout << "❌ Test failed with unknown exception\n";
        UltraAttacks::cleanup();
        return 1;
    }
}