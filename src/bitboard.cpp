// bitboard.cpp
#include "bitboard.hpp"
#include "board120.hpp"
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
    int count = 0;
    while (bb) {
        count++;
        bb &= bb - 1;  // Clear least significant bit
    }
    return count;
}

int get_lsb(Bitboard bb) {
    if (bb == 0) return -1;
    
    int pos = 0;
    if ((bb & 0xFFFFFFFF) == 0) { pos += 32; bb >>= 32; }
    if ((bb & 0xFFFF) == 0) { pos += 16; bb >>= 16; }
    if ((bb & 0xFF) == 0) { pos += 8; bb >>= 8; }
    if ((bb & 0xF) == 0) { pos += 4; bb >>= 4; }
    if ((bb & 0x3) == 0) { pos += 2; bb >>= 2; }
    if ((bb & 0x1) == 0) { pos += 1; }
    
    return pos;
}

int pop_lsb(Bitboard& bb) {
    int pos = get_lsb(bb);
    if (pos >= 0) {
        bb &= bb - 1;  // Clear the least significant bit
    }
    return pos;
}

bool is_empty(Bitboard bb) {
    return bb == 0;
}

bool is_set(Bitboard bb, int square) {
    return getBit(bb, square) != 0;
}

int sq64_to_sq120(int sq64) {
    if (sq64 < 0 || sq64 >= 64) return -1;
    
    int file = file_of_square(sq64);
    int rank = rank_of_square(sq64);
    
    // Convert to 120-square indexing
    // 120-square: A1=21, B1=22, ..., H1=28, A2=31, ..., H8=98
    return (rank + 2) * 10 + (file + 1);
}

int sq120_to_sq64(int sq120) {
    // Extract file and rank from 120-square index
    int file120 = sq120 % 10;
    int rank120 = sq120 / 10;
    
    // Check if it's a valid playable square
    if (file120 < 1 || file120 > 8 || rank120 < 2 || rank120 > 9) {
        return -1;  // Invalid square
    }
    
    // Convert to 64-square indexing
    int file64 = file120 - 1;  // 0-7
    int rank64 = rank120 - 2;  // 0-7
    
    return rank64 * 8 + file64;
}
