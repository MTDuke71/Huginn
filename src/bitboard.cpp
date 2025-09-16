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

int sq64_to_sq120(int sq64) {
    if (sq64 < 0 || sq64 >= 64) return -1;
    return MAILBOX_MAPS.to120[sq64];
}

int sq120_to_sq64(int sq120) {
    if (sq120 < 0 || sq120 >= 120) return -1;
    return MAILBOX_MAPS.to64[sq120];
}

// ============================================================================
// SLIDING PIECE ATTACK GENERATION
// ============================================================================

uint64_t generate_ray_attacks(int square, int direction, uint64_t occupancy) {
    uint64_t attacks = 0ULL;
    int file = square % 8;
    int rank = square / 8;
    
    // Calculate direction offsets correctly
    int rank_offset, file_offset;
    switch (direction) {
        case 9:   // NE
            rank_offset = 1; file_offset = 1; break;
        case 7:   // NW  
            rank_offset = 1; file_offset = -1; break;
        case -7:  // SE
            rank_offset = -1; file_offset = 1; break;
        case -9:  // SW
            rank_offset = -1; file_offset = -1; break;
        case 8:   // N
            rank_offset = 1; file_offset = 0; break;
        case -8:  // S
            rank_offset = -1; file_offset = 0; break;
        case 1:   // E
            rank_offset = 0; file_offset = 1; break;
        case -1:  // W
            rank_offset = 0; file_offset = -1; break;
        default:
            return 0ULL; // Invalid direction
    }
    
    for (int step = 1; step < 8; ++step) {
        int new_rank = rank + rank_offset * step;
        int new_file = file + file_offset * step;
        
        // Check board boundaries
        if (new_rank < 0 || new_rank >= 8 || new_file < 0 || new_file >= 8) {
            break;
        }
        
        int target_square = new_rank * 8 + new_file;
        attacks |= (1ULL << target_square);
        
        // Stop if we hit an occupied square
        if (occupancy & (1ULL << target_square)) {
            break;
        }
    }
    
    return attacks;
}

uint64_t bishop_attacks(int square, uint64_t occupancy) {
    uint64_t attacks = 0ULL;
    
    // Generate attacks in all four diagonal directions
    for (int i = 0; i < 4; ++i) {
        attacks |= generate_ray_attacks(square, BISHOP_DIRECTIONS[i], occupancy);
    }
    
    return attacks;
}

uint64_t rook_attacks(int square, uint64_t occupancy) {
    uint64_t attacks = 0ULL;
    
    // Generate attacks in all four orthogonal directions
    for (int i = 0; i < 4; ++i) {
        attacks |= generate_ray_attacks(square, ROOK_DIRECTIONS[i], occupancy);
    }
    
    return attacks;
}
