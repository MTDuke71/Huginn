/**
 * @file bitboard_attacks.hpp
 * @brief Precomputed attack tables for 64-square bitboard architecture
 * 
 * High-performance attack pattern lookups optimized for native 64-square operations.
 * Eliminates runtime calculations by precomputing all possible attack patterns.
 * 
 * @author MTDuke71
 * @version 1.0
 */
#pragma once

#include <cstdint>
#include <array>

// ============================================================================
// ATTACK PATTERN CONSTANTS
// ============================================================================

// Knight move offsets in 64-square format
constexpr int KNIGHT_OFFSETS[] = {-17, -15, -10, -6, 6, 10, 15, 17};
constexpr int NUM_KNIGHT_OFFSETS = 8;

// King move offsets in 64-square format  
constexpr int KING_OFFSETS[] = {-9, -8, -7, -1, 1, 7, 8, 9};
constexpr int NUM_KING_OFFSETS = 8;

// ============================================================================
// PRECOMPUTED ATTACK TABLES
// ============================================================================

/**
 * @brief Knight attack patterns for all 64 squares
 * 
 * knight_attacks[square] = bitboard of all squares a knight can attack from 'square'
 */
extern std::array<uint64_t, 64> knight_attacks;

/**
 * @brief King attack patterns for all 64 squares
 * 
 * king_attacks[square] = bitboard of all squares a king can attack from 'square'
 */
extern std::array<uint64_t, 64> king_attacks;

/**
 * @brief Pawn attack patterns for both colors
 * 
 * white_pawn_attacks[square] = squares a white pawn on 'square' can attack
 * black_pawn_attacks[square] = squares a black pawn on 'square' can attack
 */
extern std::array<uint64_t, 64> white_pawn_attacks;
extern std::array<uint64_t, 64> black_pawn_attacks;

// ============================================================================
// ATTACK TABLE INITIALIZATION
// ============================================================================

/**
 * @brief Initialize all precomputed attack tables
 * 
 * Must be called once during engine initialization before using attack lookups.
 * Generates optimized attack patterns for maximum performance.
 */
void init_bitboard_attacks();

// ============================================================================
// FAST ATTACK LOOKUPS
// ============================================================================

/**
 * @brief Get knight attack bitboard for a square
 * @param square 64-square index (0-63)
 * @return Bitboard of squares the knight can attack
 */
inline uint64_t get_knight_attacks(int square) {
    return knight_attacks[square];
}

/**
 * @brief Get king attack bitboard for a square
 * @param square 64-square index (0-63)
 * @return Bitboard of squares the king can attack
 */
inline uint64_t get_king_attacks(int square) {
    return king_attacks[square];
}

/**
 * @brief Get pawn attack bitboard for a square and color
 * @param square 64-square index (0-63)
 * @param is_white True for white pawns, false for black pawns
 * @return Bitboard of squares the pawn can attack
 */
inline uint64_t get_pawn_attacks(int square, bool is_white) {
    return is_white ? white_pawn_attacks[square] : black_pawn_attacks[square];
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Check if a square index is valid
 * @param square 64-square index to validate
 * @return True if square is in range [0, 63]
 */
inline bool is_valid_square_64(int square) {
    return square >= 0 && square < 64;
}

/**
 * @brief Check if a knight move is legal (within board bounds)
 * @param from_square Source square (0-63)
 * @param to_square Destination square (0-63)
 * @return True if the knight move is geometrically valid
 */
bool is_valid_knight_move(int from_square, int to_square);

/**
 * @brief Check if a king move is legal (within board bounds, adjacent)
 * @param from_square Source square (0-63)  
 * @param to_square Destination square (0-63)
 * @return True if the king move is geometrically valid
 */
bool is_valid_king_move(int from_square, int to_square);