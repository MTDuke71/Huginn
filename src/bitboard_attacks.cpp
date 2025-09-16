/**
 * @file bitboard_attacks.cpp
 * @brief Implementation of precomputed attack tables for 64-square architecture
 * 
 * Generates optimized attack patterns for knights, kings, and pawns using
 * native 64-square arithmetic without any mailbox dependencies.
 * 
 * @author MTDuke71
 * @version 1.0
 */

#include "bitboard_attacks.hpp"
#include "bitboard_position.hpp"  // For bitboard constants

// ============================================================================
// ATTACK TABLE STORAGE
// ============================================================================

std::array<uint64_t, 64> knight_attacks = {};
std::array<uint64_t, 64> king_attacks = {};
std::array<uint64_t, 64> white_pawn_attacks = {};
std::array<uint64_t, 64> black_pawn_attacks = {};

// ============================================================================
// ATTACK TABLE GENERATION
// ============================================================================

void init_bitboard_attacks() {
    // ---- Initialize knight attacks ----
    for (int square = 0; square < 64; ++square) {
        uint64_t attacks = 0ULL;
        int file = file_of_64(square);
        int rank = rank_of_64(square);
        
        // Check all 8 possible knight moves
        for (int offset : KNIGHT_OFFSETS) {
            int target_square = square + offset;
            
            if (target_square >= 0 && target_square < 64) {
                int target_file = file_of_64(target_square);
                int target_rank = rank_of_64(target_square);
                
                // Validate knight move pattern (L-shape)
                int file_diff = abs(target_file - file);
                int rank_diff = abs(target_rank - rank);
                
                if ((file_diff == 2 && rank_diff == 1) || (file_diff == 1 && rank_diff == 2)) {
                    attacks |= (1ULL << target_square);
                }
            }
        }
        
        knight_attacks[square] = attacks;
    }
    
    // ---- Initialize king attacks ----
    for (int square = 0; square < 64; ++square) {
        uint64_t attacks = 0ULL;
        int file = file_of_64(square);
        int rank = rank_of_64(square);
        
        // Check all 8 adjacent squares
        for (int offset : KING_OFFSETS) {
            int target_square = square + offset;
            
            if (target_square >= 0 && target_square < 64) {
                int target_file = file_of_64(target_square);
                int target_rank = rank_of_64(target_square);
                
                // Validate king move (max 1 square in any direction)
                int file_diff = abs(target_file - file);
                int rank_diff = abs(target_rank - rank);
                
                if (file_diff <= 1 && rank_diff <= 1) {
                    attacks |= (1ULL << target_square);
                }
            }
        }
        
        king_attacks[square] = attacks;
    }
    
    // ---- Initialize white pawn attacks ----
    for (int square = 0; square < 64; ++square) {
        uint64_t attacks = 0ULL;
        int file = file_of_64(square);
        int rank = rank_of_64(square);
        
        // White pawns attack diagonally forward (rank + 1)
        if (rank < 7) {  // Not on 8th rank
            // Left attack (file - 1, rank + 1)
            if (file > 0) {
                int target_square = square_64(file - 1, rank + 1);
                attacks |= (1ULL << target_square);
            }
            
            // Right attack (file + 1, rank + 1)
            if (file < 7) {
                int target_square = square_64(file + 1, rank + 1);
                attacks |= (1ULL << target_square);
            }
        }
        
        white_pawn_attacks[square] = attacks;
    }
    
    // ---- Initialize black pawn attacks ----
    for (int square = 0; square < 64; ++square) {
        uint64_t attacks = 0ULL;
        int file = file_of_64(square);
        int rank = rank_of_64(square);
        
        // Black pawns attack diagonally backward (rank - 1)
        if (rank > 0) {  // Not on 1st rank
            // Left attack (file - 1, rank - 1)
            if (file > 0) {
                int target_square = square_64(file - 1, rank - 1);
                attacks |= (1ULL << target_square);
            }
            
            // Right attack (file + 1, rank - 1)
            if (file < 7) {
                int target_square = square_64(file + 1, rank - 1);
                attacks |= (1ULL << target_square);
            }
        }
        
        black_pawn_attacks[square] = attacks;
    }
}

// ============================================================================
// VALIDATION FUNCTIONS
// ============================================================================

bool is_valid_knight_move(int from_square, int to_square) {
    if (!is_valid_square_64(from_square) || !is_valid_square_64(to_square)) {
        return false;
    }
    
    int from_file = file_of_64(from_square);
    int from_rank = rank_of_64(from_square);
    int to_file = file_of_64(to_square);
    int to_rank = rank_of_64(to_square);
    
    int file_diff = abs(to_file - from_file);
    int rank_diff = abs(to_rank - from_rank);
    
    return (file_diff == 2 && rank_diff == 1) || (file_diff == 1 && rank_diff == 2);
}

bool is_valid_king_move(int from_square, int to_square) {
    if (!is_valid_square_64(from_square) || !is_valid_square_64(to_square)) {
        return false;
    }
    
    int from_file = file_of_64(from_square);
    int from_rank = rank_of_64(from_square);
    int to_file = file_of_64(to_square);
    int to_rank = rank_of_64(to_square);
    
    int file_diff = abs(to_file - from_file);
    int rank_diff = abs(to_rank - from_rank);
    
    return file_diff <= 1 && rank_diff <= 1 && (file_diff + rank_diff > 0);
}