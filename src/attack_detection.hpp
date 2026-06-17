#pragma once
/**
 * @file attack_detection.hpp
 * @brief Bitboard square-attack queries used by legality, castling, and search.
 *
 * Attack detection answers the question "does this side attack this target
 * square in the current occupancy?" without generating moves. It is the hot
 * helper behind check detection, king-safety checks, castling-through-check
 * validation, and pseudo-legal move filtering.
 */

#include "chess_types.hpp"

class Position;

namespace Huginn {

/**
 * @brief Tests whether a square is attacked by any piece of a given color.
 *
 * Uses the engine's pure-bitboard attack tables and slider attack generators.
 * Pawns are handled from the target square using the opposite pawn-attack table:
 * to ask "can a white pawn attack e5?", look for white pawns on the squares from
 * which black pawns would attack e5.
 *
 * @param sq Target square index in 0..63, where a1=0 and h8=63.
 * @param pos Position whose bitboards and occupancy define attackers/blockers.
 * @param attacking_color Color whose pieces are tested as attackers.
 * @return true if at least one piece of @p attacking_color attacks @p sq.
 *
 * @pre @p sq is in range 0..63. Debug builds assert this.
 */
bool SqAttackedBB(int sq, const Position& pos, Color attacking_color);

} // namespace Huginn
