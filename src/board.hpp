// board.hpp
#pragma once
#include "position.hpp"

// Board management functions

/**
 * Reset the board to an empty, clean state
 * This is a convenience wrapper around Position::reset()
 * - Sets all offboard squares to Piece::Offboard 
 * - Sets all playable squares to Piece::None (empty)
 * - Clears all piece counts and bitboards
 * - Resets all game state variables
 * - Clears move history
 */
void reset_board(Position& pos);
