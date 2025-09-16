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
#include "board120.hpp"

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

bool verify_attack_tables() {
    // Verify some known attack patterns to ensure correctness
    
    // Knight on e4 (square 28) should attack: d2, f2, c3, g3, c5, g5, d6, f6
    // e4 = file 4, rank 3 (0-indexed) = 3*8 + 4 = 28
    uint64_t e4_knight = knight_attacks[28];
    
    // Check a few expected attack squares
    // d2 = rank 1, file 3 = 1*8 + 3 = 11
    // f6 = rank 5, file 5 = 5*8 + 5 = 45
    bool d2_attacked = (e4_knight & (1ULL << 11)) != 0;
    bool f6_attacked = (e4_knight & (1ULL << 45)) != 0;
    
    if (!d2_attacked || !f6_attacked) {
        return false;
    }
    
    // King on e4 should attack all 8 adjacent squares
    uint64_t e4_king = king_attacks[28];
    int king_attack_count = popcount(e4_king);
    if (king_attack_count != 8) {
        return false;  // King in center should attack exactly 8 squares
    }
    
    // White pawn on e4 should attack d5 and f5
    // d5 = rank 4, file 3 = 4*8 + 3 = 35
    // f5 = rank 4, file 5 = 4*8 + 5 = 37
    uint64_t e4_white_pawn = pawn_attacks[static_cast<int>(Color::White)][28];
    bool d5_attacked = (e4_white_pawn & (1ULL << 35)) != 0;
    bool f5_attacked = (e4_white_pawn & (1ULL << 37)) != 0;
    
    if (!d5_attacked || !f5_attacked) {
        return false;
    }
    
    // If we get here, basic verification passed
    return true;
}