/**
 * @file attack_detection.hpp
 * @brief Helper functions for detecting attacks on chess squares by various pieces.
 *
 * This header provides inline helper functions to determine if a given square is attacked
 * by pawns, knights, kings, or sliding pieces (rooks, bishops, queens) in a chess position.
 * It also declares the main function to check if a square is attacked by any piece of a given color.
 *
 * Functions:
 * - pawn_attacks_square: Checks if a pawn on a given square attacks a target square.
 * - knight_attacks_square: Checks if a knight on a given square attacks a target square.
 * - king_attacks_square: Checks if a king on a given square attacks a target square.
 * - sliding_attacks_rank_file: Checks if a sliding piece (rook/queen) attacks along rank or file.
 * - sliding_attacks_diagonal: Checks if a sliding piece (bishop/queen) attacks along a diagonal.
 * - SqAttacked: Main function to check if a square is attacked by any piece of a given color.
 *
 * Dependencies:
 * - position.hpp: For Position class and board state access.
 * - board120.hpp: For board representation and direction constants.
 * - chess_types.hpp: For chess type definitions (Color, File, Rank, etc.).
 * - <cmath>: For abs() function.
 */
#pragma once
#include "position.hpp"
#include "board120.hpp"
#include "chess_types.hpp"
#include <cmath>

/**
 * @brief Checks if a pawn on pawn_sq attacks target_sq.
 * @param pawn_sq The square index of the pawn.
 * @param target_sq The square index of the target square.
 * @param pawn_color The color of the pawn (White or Black).
 * @return True if the pawn attacks the target square, false otherwise.
 */
inline bool pawn_attacks_square(int pawn_sq, int target_sq, Color pawn_color) {
    if (pawn_color == Color::White) {
        // White pawn attacks NE and NW
        return (target_sq == pawn_sq + NE || target_sq == pawn_sq + NW);
    } else {
        // Black pawn attacks SE and SW
        return (target_sq == pawn_sq + SE || target_sq == pawn_sq + SW);
    }
}

/**
 * @brief Checks if a knight on knight_sq attacks target_sq.
 * @param knight_sq The square index of the knight.
 * @param target_sq The square index of the target square.
 * @return True if the knight attacks the target square, false otherwise.
 */
inline bool knight_attacks_square(int knight_sq, int target_sq) {
    for (int delta : KNIGHT_DELTAS) {
        if (knight_sq + delta == target_sq) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks if a king on king_sq attacks target_sq.
 * @param king_sq The square index of the king.
 * @param target_sq The square index of the target square.
 * @return True if the king attacks the target square, false otherwise.
 */
inline bool king_attacks_square(int king_sq, int target_sq) {
    for (int delta : KING_DELTAS) {
        if (king_sq + delta == target_sq) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks if a sliding piece (rook or queen) attacks a target square along the same rank or file.
 *
 * This function determines whether a sliding piece located at piece_sq can attack the target_sq
 * along a rank or file, considering the current board state in pos. It verifies that both squares
 * are aligned on the same rank or file, determines the direction of movement, and checks that all
 * intermediate squares between the piece and the target are unoccupied.
 *
 * @param piece_sq The square index of the sliding piece.
 * @param target_sq The square index of the target square.
 * @param pos The current board position, used to check for obstructions.
 * @return True if the sliding piece can attack the target square along the rank or file, false otherwise.
 */
inline bool sliding_attacks_rank_file(int piece_sq, int target_sq, const Position& pos) {
    // Check if on same rank or file
    File piece_file = file_of(piece_sq);
    Rank piece_rank = rank_of(piece_sq);
    File target_file = file_of(target_sq);
    Rank target_rank = rank_of(target_sq);
    
    if (piece_file != target_file && piece_rank != target_rank) {
        return false; // Not on same rank or file
    }
    
    // Determine direction
    int dir;
    if (piece_file == target_file) {
        // Same file - moving along rank
        dir = (target_rank > piece_rank) ? NORTH : SOUTH;
    } else {
        // Same rank - moving along file
        dir = (target_file > piece_file) ? EAST : WEST;
    }
    
    // Check for clear path
    int current_sq = piece_sq + dir;
    while (current_sq != target_sq && is_playable(current_sq)) {
        if (!is_none(pos.at(current_sq))) {
            return false; // Path blocked
        }
        current_sq += dir;
    }
    
    return current_sq == target_sq;
}

/**
 * @brief Checks if a sliding piece (bishop or queen) attacks a target square along a diagonal.
 *
 * This function determines whether a sliding piece located at piece_sq can attack the target_sq
 * along a diagonal, considering the current board state in pos. It verifies that both squares
 * are aligned on the same diagonal, determines the direction of movement, and checks that all
 * intermediate squares between the piece and the target are unoccupied.
 *
 * @param piece_sq The square index of the sliding piece.
 * @param target_sq The square index of the target square.
 * @param pos The current board position, used to check for obstructions.
 * @return True if the sliding piece can attack the target square along the diagonal, false otherwise.
 */
inline bool sliding_attacks_diagonal(int piece_sq, int target_sq, const Position& pos) {
    // Check if on same diagonal
    int file_diff = int(file_of(target_sq)) - int(file_of(piece_sq));
    int rank_diff = int(rank_of(target_sq)) - int(rank_of(piece_sq));
    
    if (abs(file_diff) != abs(rank_diff)) {
        return false; // Not on same diagonal
    }
    
    // Determine direction
    int dir;
    if (file_diff > 0 && rank_diff > 0) dir = NE;
    else if (file_diff < 0 && rank_diff > 0) dir = NW;
    else if (file_diff > 0 && rank_diff < 0) dir = SE;
    else dir = SW;
    
    // Check for clear path
    int current_sq = piece_sq + dir;
    while (current_sq != target_sq && is_playable(current_sq)) {
        if (!is_none(pos.at(current_sq))) {
            return false; // Path blocked
        }
        current_sq += dir;
    }
    
    return current_sq == target_sq;
}

/**
 * @brief Checks if a square is attacked by any piece of the given color.
 * @param sq The square to check for attacks.
 * @param pos The current position (for board state and occupancy).
 * @param attacking_color The color of the pieces to check for attacks.
 * @return True if the square is attacked by any piece of the given color, false otherwise.
 */
bool SqAttacked(int sq, const Position& pos, Color attacking_color);
