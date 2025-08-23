#pragma once
#include "position.hpp"
#include "board120.hpp"
#include "chess_types.hpp"

// Helper function to check if a pawn on pawn_sq attacks target_sq
inline bool pawn_attacks_square(int pawn_sq, int target_sq, Color pawn_color) {
    if (pawn_color == Color::White) {
        // White pawn attacks NE and NW
        return (target_sq == pawn_sq + NE || target_sq == pawn_sq + NW);
    } else {
        // Black pawn attacks SE and SW
        return (target_sq == pawn_sq + SE || target_sq == pawn_sq + SW);
    }
}

// Helper function to check if a knight on knight_sq attacks target_sq
inline bool knight_attacks_square(int knight_sq, int target_sq) {
    for (int delta : KNIGHT_DELTAS) {
        if (knight_sq + delta == target_sq) {
            return true;
        }
    }
    return false;
}

// Helper function to check if a king on king_sq attacks target_sq  
inline bool king_attacks_square(int king_sq, int target_sq) {
    for (int delta : KING_DELTAS) {
        if (king_sq + delta == target_sq) {
            return true;
        }
    }
    return false;
}

// Helper function to check if a sliding piece attacks along rank/file
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

// Helper function to check if a sliding piece attacks along diagonal
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

// Attack detection function declarations
// Main function to check if a square is attacked by pieces of a given color
bool SqAttacked(int sq, const Position& pos, Color attacking_color);
