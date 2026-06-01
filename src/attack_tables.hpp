/**
 * @file attack_tables.hpp
 * @brief Pre-computed attack tables for bitboard move generation
 * 
 * This file implements the attack bitboard infrastructure required for Phase 1
 * of the bitboard migration plan. It provides pre-computed attack patterns for
 * all piece types, enabling fast bitboard-based move generation and attack detection.
 * 
 * ## Architecture Overview
 * 
 * **Non-sliding pieces**: Use direct lookup tables indexed by square
 * - Knight attacks: knight_attacks[64]
 * - King attacks: king_attacks[64] 
 * - Pawn attacks: pawn_attacks[color][64]
 * 
 * **Sliding pieces**: Use magic bitboard technique (to be implemented in Phase 3)
 * - Rook attacks: rook_attacks(square, occupancy)
 * - Bishop attacks: bishop_attacks(square, occupancy)
 * - Queen attacks: combination of rook + bishop
 * 
 * ## Performance Characteristics
 * 
 * **Memory usage**: ~6KB for non-sliding pieces (minimal footprint)
 * **Lookup speed**: O(1) direct array access (fastest possible)
 * **Cache efficiency**: Linear memory layout for optimal cache usage
 * **Initialization**: One-time setup during engine startup
 * 
 * ## Usage Examples
 * 
 * ```cpp
 * // Initialize attack tables (call once at startup)
 * init_attack_tables();
 * 
 * // Get knight attacks from e4
 * uint64_t knight_moves = knight_attacks[SQ_E4];
 * 
 * // Get white pawn attacks from d4
 * uint64_t pawn_moves = pawn_attacks[WHITE][SQ_D4];
 * 
 * // Check if knight on e4 attacks f6
 * bool attacks_f6 = (knight_attacks[SQ_E4] & (1ULL << SQ_F6)) != 0;
 * ```
 * 
 * @author MTDuke71
 * @version 1.0
 * @see bitboard.hpp for underlying bitboard operations
 * @see attack_detection.hpp for current piece-list based attack detection
 */

#pragma once

#include <cstdint>
#include "chess_types.hpp"
#include "bitboard.hpp"

// ============================================================================
// ATTACK TABLE DECLARATIONS
// ============================================================================

/// Pre-computed knight attack patterns for each square (64 entries)
extern uint64_t knight_attacks[64];

/// Pre-computed king attack patterns for each square (64 entries) 
extern uint64_t king_attacks[64];

/// Pre-computed pawn attack patterns [color][square] (2x64 entries)
/// pawn_attacks[WHITE][square] = squares that a white pawn on 'square' attacks
/// pawn_attacks[BLACK][square] = squares that a black pawn on 'square' attacks
extern uint64_t pawn_attacks[2][64];

// ============================================================================
// SLIDING PIECE ATTACK FUNCTIONS (Implemented in bitboard.hpp/cpp)
// ============================================================================

// Note: Sliding piece attack functions are already declared in bitboard.hpp:
// - uint64_t rook_attacks(int square, uint64_t occupancy);
// - uint64_t bishop_attacks(int square, uint64_t occupancy);  
// - inline uint64_t queen_attacks(int square, uint64_t occupancy);
//
// These will be implemented with magic bitboards in Phase 3 of the migration.

// ============================================================================
// INITIALIZATION AND UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Initialize all attack tables (call once at engine startup)
 * 
 * This function pre-computes all attack patterns for non-sliding pieces.
 * It must be called before using any of the attack table arrays.
 * 
 * **Initialization order:**
 * 1. Knight attack patterns for all 64 squares
 * 2. King attack patterns for all 64 squares
 * 3. Pawn attack patterns for both colors and all 64 squares
 * 
 * **Performance**: Takes ~1ms to initialize, saves thousands of cycles per lookup
 */
void init_attack_tables();

// ============================================================================
// INLINE CONVENIENCE FUNCTIONS
// ============================================================================

