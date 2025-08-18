// bitboard.hpp
#pragma once
#include <cstdint>
#include <iostream>
#include "chess_types.hpp"
#include "board120.hpp"  // For MAILBOX_MAPS access in SQ64/SQ120 macros

// Bitboard type - represents 64 squares using bits
using Bitboard = uint64_t;

// ---- Bitboard Macros ----
// Using pre-computed masks for better performance than runtime bit shifts
#define setBit(bb, sq)   ((bb) |= BIT_MASK[sq])
#define popBit(bb, sq)   ((bb) &= CLEAR_MASK[sq])
#define getBit(bb, sq)   ((bb) & BIT_MASK[sq])

// Alternative names for clarity
#define addBit(bb, sq)   setBit(bb, sq)
#define PopBit(bb, sq)   popBit(bb, sq)

// Additional alias macros
#define SETBIT(bb, sq)   setBit(bb, sq)
#define CLRBIT(bb, sq)   popBit(bb, sq)

// Utility macros for common operations
#define POP(bb)          pop_lsb(bb)     // Pop and return least significant bit
#define CNT(bb)          countBit(bb)    // Count number of set bits

// ---- Bitboard Constants ----
constexpr Bitboard EMPTY_BB = 0ULL;
constexpr Bitboard FULL_BB = 0xFFFFFFFFFFFFFFFFULL;

// Pre-computed bit masks for each square (0-63) - more efficient than runtime shifts
constexpr Bitboard BIT_MASK[64] = {
    0x0000000000000001ULL, 0x0000000000000002ULL, 0x0000000000000004ULL, 0x0000000000000008ULL,
    0x0000000000000010ULL, 0x0000000000000020ULL, 0x0000000000000040ULL, 0x0000000000000080ULL,
    0x0000000000000100ULL, 0x0000000000000200ULL, 0x0000000000000400ULL, 0x0000000000000800ULL,
    0x0000000000001000ULL, 0x0000000000002000ULL, 0x0000000000004000ULL, 0x0000000000008000ULL,
    0x0000000000010000ULL, 0x0000000000020000ULL, 0x0000000000040000ULL, 0x0000000000080000ULL,
    0x0000000000100000ULL, 0x0000000000200000ULL, 0x0000000000400000ULL, 0x0000000000800000ULL,
    0x0000000001000000ULL, 0x0000000002000000ULL, 0x0000000004000000ULL, 0x0000000008000000ULL,
    0x0000000010000000ULL, 0x0000000020000000ULL, 0x0000000040000000ULL, 0x0000000080000000ULL,
    0x0000000100000000ULL, 0x0000000200000000ULL, 0x0000000400000000ULL, 0x0000000800000000ULL,
    0x0000001000000000ULL, 0x0000002000000000ULL, 0x0000004000000000ULL, 0x0000008000000000ULL,
    0x0000010000000000ULL, 0x0000020000000000ULL, 0x0000040000000000ULL, 0x0000080000000000ULL,
    0x0000100000000000ULL, 0x0000200000000000ULL, 0x0000400000000000ULL, 0x0000800000000000ULL,
    0x0001000000000000ULL, 0x0002000000000000ULL, 0x0004000000000000ULL, 0x0008000000000000ULL,
    0x0010000000000000ULL, 0x0020000000000000ULL, 0x0040000000000000ULL, 0x0080000000000000ULL,
    0x0100000000000000ULL, 0x0200000000000000ULL, 0x0400000000000000ULL, 0x0800000000000000ULL,
    0x1000000000000000ULL, 0x2000000000000000ULL, 0x4000000000000000ULL, 0x8000000000000000ULL
};

// Pre-computed inverted bit masks for clearing bits - more efficient than runtime ~(1ULL << sq)
constexpr Bitboard CLEAR_MASK[64] = {
    0xFFFFFFFFFFFFFFFEULL, 0xFFFFFFFFFFFFFFFDULL, 0xFFFFFFFFFFFFFFFBULL, 0xFFFFFFFFFFFFFFF7ULL,
    0xFFFFFFFFFFFFFFEFULL, 0xFFFFFFFFFFFFFFDFULL, 0xFFFFFFFFFFFFFFBFULL, 0xFFFFFFFFFFFFFF7FULL,
    0xFFFFFFFFFFFFFEFFULL, 0xFFFFFFFFFFFFFDFFULL, 0xFFFFFFFFFFFFFBFFULL, 0xFFFFFFFFFFFFF7FFULL,
    0xFFFFFFFFFFFFEFFFULL, 0xFFFFFFFFFFFFDFFFULL, 0xFFFFFFFFFFFFBFFFULL, 0xFFFFFFFFFFFF7FFFULL,
    0xFFFFFFFFFFFEFFFFULL, 0xFFFFFFFFFFFDFFFFULL, 0xFFFFFFFFFFFBFFFFULL, 0xFFFFFFFFFFF7FFFFULL,
    0xFFFFFFFFFFEFFFFFULL, 0xFFFFFFFFFFDFFFFFULL, 0xFFFFFFFFFFBFFFFFULL, 0xFFFFFFFFFF7FFFFFULL,
    0xFFFFFFFFFEFFFFFFULL, 0xFFFFFFFFFDFFFFFFULL, 0xFFFFFFFFFBFFFFFFULL, 0xFFFFFFFFF7FFFFFFULL,
    0xFFFFFFFFEFFFFFFFULL, 0xFFFFFFFFDFFFFFFFULL, 0xFFFFFFFFBFFFFFFFULL, 0xFFFFFFFF7FFFFFFFULL,
    0xFFFFFFFEFFFFFFFFULL, 0xFFFFFFFDFFFFFFFFULL, 0xFFFFFFFBFFFFFFFFULL, 0xFFFFFFF7FFFFFFFFULL,
    0xFFFFFFEFFFFFFFFFULL, 0xFFFFFFDFFFFFFFFFULL, 0xFFFFFFBFFFFFFFFFULL, 0xFFFFFF7FFFFFFFFFULL,
    0xFFFFFEFFFFFFFFFFULL, 0xFFFFFDFFFFFFFFFFULL, 0xFFFFFBFFFFFFFFFFULL, 0xFFFFF7FFFFFFFFFFULL,
    0xFFFFEFFFFFFFFFFFULL, 0xFFFFDFFFFFFFFFFFULL, 0xFFFFBFFFFFFFFFFFULL, 0xFFFF7FFFFFFFFFFFULL,
    0xFFFEFFFFFFFFFFFFULL, 0xFFFDFFFFFFFFFFFFULL, 0xFFFBFFFFFFFFFFFFULL, 0xFFF7FFFFFFFFFFFFULL,
    0xFFEFFFFFFFFFFFFFULL, 0xFFDFFFFFFFFFFFFFULL, 0xFFBFFFFFFFFFFFFFULL, 0xFF7FFFFFFFFFFFFFULL,
    0xFEFFFFFFFFFFFFFFULL, 0xFDFFFFFFFFFFFFFFULL, 0xFBFFFFFFFFFFFFFFULL, 0xF7FFFFFFFFFFFFFFULL,
    0xEFFFFFFFFFFFFFFFULL, 0xDFFFFFFFFFFFFFFFULL, 0xBFFFFFFFFFFFFFFFULL, 0x7FFFFFFFFFFFFFFFULL
};

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

// Convenient macros for square conversion (direct MAILBOX_MAPS access)
#define SQ64(sq120)  (MAILBOX_MAPS.to64[sq120])    // Convert sq120 → sq64
#define SQ120(sq64)  (MAILBOX_MAPS.to120[sq64])    // Convert sq64 → sq120
