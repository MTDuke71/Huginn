/**
 * @file attack_tables.cpp
 * @brief Implementation of pre-computed attack table generation
 * 
 * This file generates the attack bitboards for non-sliding pieces at startup.
 * The initialization functions use traditional move pattern logic to generate
 * the bitboard representations, creating a fast lookup table for runtime use.
 * 
 * ## Implementation Strategy
 * 
 * **Knight patterns**: Use traditional +/- 2,1 and +/- 1,2 offset pattern
 * **King patterns**: Use 8-direction offset pattern for adjacent squares  
 * **Pawn patterns**: Use diagonal capture patterns with color-specific direction
 * 
 * ## Performance Design
 * 
 * - **One-time cost**: Initialization happens once at engine startup
 * - **Runtime benefit**: O(1) lookup replaces O(k) pattern calculation
 * - **Memory efficiency**: ~6KB total for all non-sliding attack tables
 * - **Cache friendly**: Linear arrays with optimal memory layout
 * 
 * @author MTDuke71
 * @version 1.0
 * @see attack_tables.hpp for interface documentation
 */

#include "attack_tables.hpp"
#include "bitboard.hpp"
#include "square.hpp"

// ============================================================================
// ATTACK TABLE STORAGE
// ============================================================================

/// Knight attack table: indexed by square (0-63)
uint64_t knight_attacks[64];

/// King attack table: indexed by square (0-63)
uint64_t king_attacks[64];

/// Pawn attack table: indexed by [color][square]
uint64_t pawn_attacks[2][64];

// ============================================================================
// KNIGHT ATTACK GENERATION
// ============================================================================

/**
 * @brief Generate knight attack bitboard for a specific square
 * @param square The square to generate knight attacks for (0-63)
 * @return Bitboard representing all squares a knight on 'square' can attack
 */
static uint64_t generate_knight_attacks(int square) {
    uint64_t attacks = 0ULL;
    
    // Knight moves: 8 possible L-shaped moves
    // Each move is +/- 2 in one direction, +/- 1 in the other
    const int knight_moves[8][2] = {
        {-2, -1}, {-2, +1},  // Up 2, left/right 1
        {-1, -2}, {-1, +2},  // Up 1, left/right 2  
        {+1, -2}, {+1, +2},  // Down 1, left/right 2
        {+2, -1}, {+2, +1}   // Down 2, left/right 1
    };
    
    int file = square % 8;
    int rank = square / 8;
    
    for (int i = 0; i < 8; i++) {
        int new_rank = rank + knight_moves[i][0];
        int new_file = file + knight_moves[i][1];
        
        // Check if the new position is within board bounds
        if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
            int target_square = new_rank * 8 + new_file;
            setBit(attacks, target_square);
        }
    }
    
    return attacks;
}

// ============================================================================
// KING ATTACK GENERATION  
// ============================================================================

/**
 * @brief Generate king attack bitboard for a specific square
 * @param square The square to generate king attacks for (0-63)
 * @return Bitboard representing all squares a king on 'square' can attack
 */
static uint64_t generate_king_attacks(int square) {
    uint64_t attacks = 0ULL;
    
    // King moves: 8 possible adjacent squares
    const int king_moves[8][2] = {
        {-1, -1}, {-1,  0}, {-1, +1},  // Up row: left, center, right
        { 0, -1},           { 0, +1},  // Same row: left, right
        {+1, -1}, {+1,  0}, {+1, +1}   // Down row: left, center, right
    };
    
    int file = square % 8;
    int rank = square / 8;
    
    for (int i = 0; i < 8; i++) {
        int new_rank = rank + king_moves[i][0];
        int new_file = file + king_moves[i][1];
        
        // Check if the new position is within board bounds
        if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
            int target_square = new_rank * 8 + new_file;
            setBit(attacks, target_square);
        }
    }
    
    return attacks;
}

// ============================================================================
// PAWN ATTACK GENERATION
// ============================================================================

/**
 * @brief Generate pawn attack bitboard for a specific square and color
 * @param square The square to generate pawn attacks for (0-63)
 * @param color The color of the pawn (WHITE or BLACK)
 * @return Bitboard representing squares the pawn can capture
 */
static uint64_t generate_pawn_attacks(int square, Color color) {
    uint64_t attacks = 0ULL;
    
    int file = square % 8;
    int rank = square / 8;
    
    // Determine attack direction based on color
    int rank_direction = (color == Color::White) ? +1 : -1;  // White moves up, Black moves down
    int attack_rank = rank + rank_direction;
    
    // Check if attack rank is within bounds
    if (attack_rank >= 0 && attack_rank < 8) {
        // Left diagonal capture
        if (file > 0) {  // Not on A-file
            int left_attack_square = attack_rank * 8 + (file - 1);
            setBit(attacks, left_attack_square);
        }
        
        // Right diagonal capture  
        if (file < 7) {  // Not on H-file
            int right_attack_square = attack_rank * 8 + (file + 1);
            setBit(attacks, right_attack_square);
        }
    }
    
    return attacks;
}

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

void init_attack_tables() {
    // Initialize knight attack table
    for (int square = 0; square < 64; square++) {
        knight_attacks[square] = generate_knight_attacks(square);
    }
    
    // Initialize king attack table
    for (int square = 0; square < 64; square++) {
        king_attacks[square] = generate_king_attacks(square);
    }
    
    // Initialize pawn attack tables for both colors
    for (int square = 0; square < 64; square++) {
        pawn_attacks[static_cast<int>(Color::White)][square] = generate_pawn_attacks(square, Color::White);
        pawn_attacks[static_cast<int>(Color::Black)][square] = generate_pawn_attacks(square, Color::Black);
    }
}

