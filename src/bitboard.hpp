// bitboard.hpp
#pragma once
#include <cstdint>
#include <iostream>
#include "chess_types.hpp"

// Bitboard type - represents 64 squares using bits
using Bitboard = uint64_t;

// ---- Bitboard Macros ----
#define setBit(bb, sq)   ((bb) |= (1ULL << (sq)))
#define popBit(bb, sq)   ((bb) &= ~(1ULL << (sq)))
#define getBit(bb, sq)   ((bb) & (1ULL << (sq)))

// Alternative names for clarity
#define addBit(bb, sq)   setBit(bb, sq)
#define PopBit(bb, sq)   popBit(bb, sq)

// Utility macros for common operations
#define POP(bb)          pop_lsb(bb)     // Pop and return least significant bit
#define CNT(bb)          countBit(bb)    // Count number of set bits

// ---- Bitboard Constants ----
constexpr Bitboard EMPTY_BB = 0ULL;
constexpr Bitboard FULL_BB = 0xFFFFFFFFFFFFFFFFULL;

// File bitboards (A-H files)
constexpr Bitboard FILE_A = 0x0101010101010101ULL;
constexpr Bitboard FILE_B = 0x0202020202020202ULL;
constexpr Bitboard FILE_C = 0x0404040404040404ULL;
constexpr Bitboard FILE_D = 0x0808080808080808ULL;
constexpr Bitboard FILE_E = 0x1010101010101010ULL;
constexpr Bitboard FILE_F = 0x2020202020202020ULL;
constexpr Bitboard FILE_G = 0x4040404040404040ULL;
constexpr Bitboard FILE_H = 0x8080808080808080ULL;

// Rank bitboards (1-8 ranks)
constexpr Bitboard RANK_1 = 0x00000000000000FFULL;
constexpr Bitboard RANK_2 = 0x000000000000FF00ULL;
constexpr Bitboard RANK_3 = 0x0000000000FF0000ULL;
constexpr Bitboard RANK_4 = 0x00000000FF000000ULL;
constexpr Bitboard RANK_5 = 0x000000FF00000000ULL;
constexpr Bitboard RANK_6 = 0x0000FF0000000000ULL;
constexpr Bitboard RANK_7 = 0x00FF000000000000ULL;
constexpr Bitboard RANK_8 = 0xFF00000000000000ULL;

// Array access for files and ranks
constexpr Bitboard FILE_BB[8] = {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

constexpr Bitboard RANK_BB[8] = {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
};

// ---- Function Declarations ----

// Print bitboard with rank/file borders
// 'x' for occupied squares, '-' for empty squares
// Rank 8 at top, Rank 1 at bottom
void printBitboard(Bitboard bb);

// Print bitboard with custom character
void printBitboard(Bitboard bb, char occupied_char, char empty_char = '-');

// Utility functions
int popcount(Bitboard bb);           // Count number of set bits (optimized with __builtin_popcountll)
int countBit(Bitboard bb);           // Count number of set bits - alias for popcount (optimized with __builtin_popcountll)
int get_lsb(Bitboard bb);           // Get least significant bit position (optimized with __builtin_ctzll)
int pop_lsb(Bitboard& bb);          // Pop and return least significant bit (optimized with __builtin_ctzll)
bool is_empty(Bitboard bb);         // Check if bitboard is empty
bool is_set(Bitboard bb, int square); // Check if specific square is set

// Square conversion (0-63 standard bitboard indexing)
// a1=0, b1=1, ..., h1=7, a2=8, ..., h8=63
constexpr int square_from_file_rank(int file, int rank) {
    return rank * 8 + file;
}

constexpr int file_of_square(int square) {
    return square & 7;
}

constexpr int rank_of_square(int square) {
    return square >> 3;
}

// Convert between different square representations
// Uses pre-computed MAILBOX_MAPS arrays for consistency and reliability
int sq64_to_sq120(int sq64);        // Convert 64-square to 120-square index
int sq120_to_sq64(int sq120);       // Convert 120-square to 64-square index
