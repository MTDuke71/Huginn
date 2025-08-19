#pragma once
#include "position.hpp"
#include <string>
#include <cassert>

namespace Debug {

/**
 * Comprehensive position validation function that compares a position
 * against a FEN string and validates all internal data structures for consistency.
 * 
 * @param pos The position to validate
 * @param expected_fen The FEN string representing the expected position
 * @return true if all validations pass, false otherwise (with assertions pointing out errors)
 */
bool validate_position_consistency(const Position& pos, const std::string& expected_fen);

/**
 * Validates that all bitboards are consistent with the board array
 * 
 * @param pos The position to validate
 * @return true if bitboards match board array
 */
bool validate_bitboards_consistency(const Position& pos);

/**
 * Validates that piece counts match the actual pieces on the board
 * 
 * @param pos The position to validate
 * @return true if piece counts are consistent
 */
bool validate_piece_counts_consistency(const Position& pos);

/**
 * Validates that piece lists contain the correct squares for each piece
 * 
 * @param pos The position to validate
 * @return true if piece lists are consistent
 */
bool validate_piece_lists_consistency(const Position& pos);

/**
 * Validates that material scores match the actual pieces on the board
 * 
 * @param pos The position to validate
 * @return true if material scores are consistent
 */
bool validate_material_scores_consistency(const Position& pos);

/**
 * Validates that king squares are correctly tracked
 * 
 * @param pos The position to validate
 * @return true if king squares are consistent
 */
bool validate_king_squares_consistency(const Position& pos);

/**
 * Validates that the Zobrist hash matches the current position
 * 
 * @param pos The position to validate
 * @return true if Zobrist hash is consistent
 */
bool validate_zobrist_consistency(const Position& pos);

/**
 * Validates that en passant square is on the correct rank for the side to move
 * 
 * @param pos The position to validate
 * @return true if en passant square is valid
 */
bool validate_en_passant_consistency(const Position& pos);

/**
 * Validates that castling rights are consistent with piece positions
 * Kings and rooks must be on their starting squares if castling is allowed
 * 
 * @param pos The position to validate
 * @return true if castling rights are consistent with piece positions
 */
bool validate_castling_consistency(const Position& pos);

} // namespace Debug
