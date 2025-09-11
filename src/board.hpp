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

/**
 * Print a chess position in a visual ASCII format
 * Displays the board with pieces, coordinates, and game state information
 * - Shows pieces using FEN notation (K/Q/R/B/N/P for white, k/q/r/b/n/p for black)
 * - Empty squares shown as dots (.)
 * - Includes rank/file labels and borders
 * - Shows side to move, castling rights, en passant, move counters, piece counts
 */
void print_position(const Position& pos);
