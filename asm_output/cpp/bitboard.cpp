// bitboard.cpp
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

int pop_lsb(Bitboard& bb) {
    if (bb == 0) return -1;
    int index = __builtin_ctzll(bb);   // Cross-platform count trailing zeros via bit_utils.hpp
    bb &= bb - 1;                      // clears the least significant 1 bit
    return index;
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
