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
#include "magic_bitboards.hpp"  // BACKLOG #24: real magic sliders
#include <iostream>
#include <iomanip>

/// @brief Print @p bb as an 8×8 board (rank 8 top), 'x' = set, '-' = clear. Debug aid.
void printBitboard(Bitboard bb) {
    printBitboard(bb, 'x', '-');
}

/// @brief printBitboard with caller-chosen occupied/empty glyphs.
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

/// @brief Number of set bits in @p bb (hardware popcount).
int popcount(Bitboard bb) {
    return __builtin_popcountll(bb);  // Cross-platform popcount via bit_utils.hpp
}

/// @brief Alias of popcount() (legacy name).
int countBit(Bitboard bb) {
    return __builtin_popcountll(bb);  // Alias for popcount - efficiently count set bits
}

/// @brief Index (0-63) of the least-significant set bit, or -1 if @p bb is empty.
int get_lsb(Bitboard bb) {
    if (bb == 0) return -1;
    return __builtin_ctzll(bb);   // Cross-platform count trailing zeros via bit_utils.hpp
}

/// @brief True if @p bb has no bits set.
bool is_empty(Bitboard bb) {
    return bb == 0;
}

/// @brief True if @p square (0-63) is set in @p bb.
bool is_set(Bitboard bb, int square) {
    return getBit(bb, square) != 0;
}

// ============================================================================
// SLIDING PIECE ATTACK GENERATION
// ============================================================================

// BACKLOG #24: bishop_attacks/rook_attacks delegate to real magic bitboards
// (src/magic_bitboards.{hpp,cpp}). The old ray-walker was removed in the #26
// follow-up — magic init has its own ray walker for table population.
/// @brief Bishop attack set from @p square given board @p occupancy (magic lookup).
uint64_t bishop_attacks(int square, uint64_t occupancy) {
    return Magic::magic_bishop_attacks(square, occupancy);
}

/// @brief Rook attack set from @p square given board @p occupancy (magic lookup).
uint64_t rook_attacks(int square, uint64_t occupancy) {
    return Magic::magic_rook_attacks(square, occupancy);
}
