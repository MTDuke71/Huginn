/**
 * @file pawn_lookup_tables.hpp
 * @brief Pawn attack bitboard table for bitboard move generation
 *
 * Pre-computed pawn attack bitboards indexed by [Color][sq64]. The
 * production bitboard movegen ([src/movegen_bb.cpp](movegen_bb.cpp))
 * masks these against the enemy occupancy for O(1) pawn-capture and
 * en-passant attacker generation.
 *
 * History: this header used to also expose five [2][120]
 * mailbox-indexed move tables (forward / double / capture / move-mask)
 * and their get_pawn_* accessors. They had no callers after the
 * S_MOVE 120-to-64 migration and were deleted alongside the other dead
 * mailbox-120 structures; only the sq64 attack table remains.
 *
 * @author MTDuke71
 * @version 2.0
 */

#pragma once

#include <cstdint>
#include "chess_types.hpp"

namespace PawnLookupTables {

// Bitboard attack tables for bitboard-based move generation
// PAWN_ATTACKS[color][square64] = bitboard of attacked squares
extern uint64_t PAWN_ATTACKS[2][64];

/**
 * @brief Initialize the pawn attack bitboard table
 *
 * Pre-computes the squares attacked by a pawn of each color from every
 * sq64. Must be called once during engine initialization before any
 * move generation.
 */
void initialize_pawn_tables();

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
