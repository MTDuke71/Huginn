/**
 * @file pawn_lookup_tables.hpp
 * @brief Pawn move lookup tables for optimized move generation
 * 
 * Pre-computed lookup tables for pawn moves to eliminate runtime arithmetic
 * and improve performance. Works alongside pawn_optimizations.hpp to further
 * reduce the 20.3% time overhead from pawn move generation.
 * 
 * ## Lookup Table Structure
 * - PAWN_FORWARD_MOVES: Single forward move destination for each square
 * - PAWN_DOUBLE_MOVES: Double forward move destination from starting ranks
 * - PAWN_CAPTURE_LEFT: Left diagonal capture destinations
 * - PAWN_CAPTURE_RIGHT: Right diagonal capture destinations
 * - PAWN_MOVE_COUNT: Number of possible moves from each square (for iteration)
 * 
 * ## Usage Pattern
 * Instead of calculating `from + direction + WEST`, use:
 * `PAWN_CAPTURE_LEFT[color][from]` for O(1) lookup
 * 
 * ## Memory Layout
 * Tables are indexed by [Color][Square] for cache-friendly access patterns.
 * Invalid moves are marked with INVALID_SQUARE (-1).
 * 
 * @author MTDuke71
 * @version 1.0
 * @see pawn_optimizations.hpp for the main pawn move generation logic
 */

#pragma once

#include "chess_types.hpp"
#include "board120.hpp"

namespace PawnLookupTables {

// Invalid square marker for impossible moves
constexpr int INVALID_SQUARE = -1;

// Forward move destinations (single step)
// PAWN_FORWARD_MOVES[color][from_square] = to_square
extern int (*PAWN_FORWARD_MOVES)[120];

// Double move destinations from starting ranks
// PAWN_DOUBLE_MOVES[color][from_square] = to_square (or INVALID_SQUARE)
extern int (*PAWN_DOUBLE_MOVES)[120];

// Left diagonal capture destinations
// PAWN_CAPTURE_LEFT[color][from_square] = to_square (or INVALID_SQUARE)
extern int (*PAWN_CAPTURE_LEFT)[120];

// Right diagonal capture destinations  
// PAWN_CAPTURE_RIGHT[color][from_square] = to_square (or INVALID_SQUARE)
extern int (*PAWN_CAPTURE_RIGHT)[120];

// Count of possible move types from each square (for loop optimization)
// Bit flags: 1=forward, 2=double, 4=capture_left, 8=capture_right
extern int (*PAWN_MOVE_MASK)[120];

// Bitboard attack tables for bitboard-based move generation
// PAWN_ATTACKS[color][square64] = bitboard of attacked squares
extern uint64_t PAWN_ATTACKS[2][64];

/**
 * @brief Initialize all pawn lookup tables
 * 
 * Pre-computes all possible pawn move destinations for both colors.
 * Must be called once during engine initialization before any move generation.
 * 
 * ## Table Population
 * - White pawns: direction = NORTH (-10)
 * - Black pawns: direction = SOUTH (+10)
 * - Validates all moves with IS_PLAYABLE() before storing
 * - Handles edge cases (file boundaries, board edges)
 * 
 * @note This function allocates the lookup table memory and populates it.
 *       Tables remain valid for the lifetime of the program.
 */
void initialize_pawn_tables();

/**
 * @brief Get forward move destination for a pawn
 * @param color The color of the pawn (White or Black)
 * @param from_square The source square (120-square mailbox format)
 * @return Destination square or INVALID_SQUARE if move is not possible
 */
inline int get_pawn_forward_move(Color color, int from_square) {
    return PAWN_FORWARD_MOVES[int(color)][from_square];
}

/**
 * @brief Get double move destination for a pawn from starting rank
 * @param color The color of the pawn (White or Black)  
 * @param from_square The source square (120-square mailbox format)
 * @return Destination square or INVALID_SQUARE if double move is not possible
 */
inline int get_pawn_double_move(Color color, int from_square) {
    return PAWN_DOUBLE_MOVES[int(color)][from_square];
}

/**
 * @brief Get left diagonal capture destination for a pawn
 * @param color The color of the pawn (White or Black)
 * @param from_square The source square (120-square mailbox format)  
 * @return Destination square or INVALID_SQUARE if capture is not possible
 */
inline int get_pawn_capture_left(Color color, int from_square) {
    return PAWN_CAPTURE_LEFT[int(color)][from_square];
}

/**
 * @brief Get right diagonal capture destination for a pawn
 * @param color The color of the pawn (White or Black)
 * @param from_square The source square (120-square mailbox format)
 * @return Destination square or INVALID_SQUARE if capture is not possible  
 */
inline int get_pawn_capture_right(Color color, int from_square) {
    return PAWN_CAPTURE_RIGHT[int(color)][from_square];
}

/**
 * @brief Get move type mask for a pawn square
 * @param color The color of the pawn (White or Black)
 * @param from_square The source square (120-square mailbox format)
 * @return Bit mask indicating possible move types (1=forward, 2=double, 4=left, 8=right)
 */
inline int get_pawn_move_mask(Color color, int from_square) {
    return PAWN_MOVE_MASK[int(color)][from_square];
}

// Move type bit flags for PAWN_MOVE_MASK
constexpr int FORWARD_MOVE_FLAG = 1;
constexpr int DOUBLE_MOVE_FLAG = 2;
constexpr int LEFT_CAPTURE_FLAG = 4;
constexpr int RIGHT_CAPTURE_FLAG = 8;

/**
 * @brief Get pawn attack bitboard for a square
 * @param color The color of the pawn (White or Black)
 * @param square The square in 64-square format (0-63)
 * @return Bitboard of squares attacked by the pawn
 */
inline uint64_t get_pawn_attacks(Color color, int square) {
    return PAWN_ATTACKS[int(color)][square];
}

} // namespace PawnLookupTables