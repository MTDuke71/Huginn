/**
 * @file bitboard.cpp
 * @brief Implementation of bitboard utility functions
 * 
 * Provides implementations for bitboard printing, visualization, and debugging
 * utilities. These functions are primarily used for development, testing, and
 * analysis rather than core engine performance, so they prioritize clarity
 * and functionality over speed.
 * 
 * ## Utility Functions
 * - **Bitboard Printing**: Visual representation of bitboard state
 * - **Custom Characters**: Configurable display characters for occupied/empty squares
 * - **Chess Notation**: Standard algebraic notation for coordinates
 * - **Debug Support**: Clear visualization for development and debugging
 * 
 * @author MTDuke71
 * @version 1.2
 * @see bitboard.hpp for bitboard type definitions and core operations
 */
#include "bitboard.hpp"
#include "bit_utils.hpp"   // Cross-platform bit manipulation utilities
#include "board120.hpp"    // For MAILBOX_MAPS conversion arrays
#include "magic_bitboards.hpp"  // BACKLOG #24: real magic sliders
#include <iostream>
#include <iomanip>

void printBitboard(Bitboard bb) {
    printBitboard(bb, 'x', '-');
}

void printBitboard(Bitboard bb, char occupied_char, char empty_char) {
    std::cout << "\n";
    std::cout << "    a b c d e f g h\n";
    std::cout << "  +---------------+\n";
    
    // Print from rank 8 (top) to rank 1 (bottom)
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << (rank + 1) << " | ";
        
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;  // 64-square indexing
            
            if (getBit(bb, square)) {
                std::cout << occupied_char << " ";
            } else {
                std::cout << empty_char << " ";
            }
        }
        
        std::cout << "| " << (rank + 1) << "\n";
    }
    
    std::cout << "  +---------------+\n";
    std::cout << "    a b c d e f g h\n\n";
}

int popcount(Bitboard bb) {
    return __builtin_popcountll(bb);  // Cross-platform popcount via bit_utils.hpp
}

int countBit(Bitboard bb) {
    return __builtin_popcountll(bb);  // Alias for popcount - efficiently count set bits
}

int get_lsb(Bitboard bb) {
    if (bb == 0) return -1;
    return __builtin_ctzll(bb);   // Cross-platform count trailing zeros via bit_utils.hpp
}

int get_msb(Bitboard bb) {
    if (bb == 0) return -1;
    return 63 - __builtin_clzll(bb);   // Cross-platform count leading zeros to find MSB
}

bool is_empty(Bitboard bb) {
    return bb == 0;
}

bool is_set(Bitboard bb, int square) {
    return getBit(bb, square) != 0;
}

// ============================================================================
// SLIDING PIECE ATTACK GENERATION
// ============================================================================

// BACKLOG #24: bishop_attacks/rook_attacks delegate to real magic
// bitboards (src/magic_bitboards.{hpp,cpp}). The old ray-walker
// (generate_ray_attacks) was removed in #26 follow-up — the magic init
// has its own self-contained ray walker for table population +
// verification, so the bitboard.cpp ray walker had no remaining callers.
uint64_t bishop_attacks(int square, uint64_t occupancy) {
    return Magic::magic_bishop_attacks(square, occupancy);
}

uint64_t rook_attacks(int square, uint64_t occupancy) {
    return Magic::magic_rook_attacks(square, occupancy);
}
