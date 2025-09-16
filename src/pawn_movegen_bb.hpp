/**
 * @file pawn_movegen_bb.hpp
 * @brief Phase 2: Bitboard-based pawn move generation
 * 
 * Implements the pawn move generation migration from Phase 2 of the bitboard
 * migration plan. This module replaces piece-list iteration with efficient
 * bitboard shift operations for significant performance improvements.
 * 
 * ## Performance Target (Phase 2)
 * - **Goal**: 15-25% improvement in pawn move generation
 * - **Current impact**: Pawn moves are 20.3% of total generation time
 * - **Strategy**: Use bitboard shifts instead of piece list iteration
 * 
 * ## Implementation Strategy
 * 
 * **Single pawn pushes**: `(our_pawns << 8) & empty` for white
 * **Double pawn pushes**: `(single_pushes << 8) & empty & RANK_4` for white
 * **Left captures**: `(our_pawns << 7) & enemies & ~FILE_H` for white
 * **Right captures**: `(our_pawns << 9) & enemies & ~FILE_A` for white
 * **Promotions**: Handle 8th rank moves with 4 piece type variants
 * **En passant**: Special capture handling with bitboard operations
 * 
 * ## Migration Plan Pattern
 * 
 * ```cpp
 * void generate_pawn_moves_bb(const Position& pos, S_MOVELIST& list) {
 *     Color us = pos.side;
 *     uint64_t our_pawns = pos.piece_bitboards[size_t(us)][size_t(PieceType::Pawn)];
 *     uint64_t empty = ~pos.occupied_bitboard;
 *     uint64_t enemies = pos.color_bitboards[1 - size_t(us)];
 *     
 *     // Single pawn pushes with bitboard shifts...
 * }
 * ```
 * 
 * @author MTDuke71
 * @version 1.0
 * @see bitboard_migration_plan.md for complete Phase 2 specification
 */

#pragma once

#include <cstdint>
#include "chess_types.hpp"
#include "position.hpp"
#include "move.hpp"
#include "bitboard.hpp"

// Forward declaration
struct S_MOVELIST;

// ============================================================================
// PHASE 2 BITBOARD PAWN MOVE GENERATION
// ============================================================================

/**
 * @brief Generate pawn moves using bitboard operations (Phase 2 migration)
 * @param pos The current position with bitboard state
 * @param list Move list to append generated moves to
 * 
 * This function implements the exact pattern from the migration plan using
 * bitboard shift operations instead of piece list iteration. Targets 15-25%
 * improvement over the current piece-list based approach.
 * 
 * **Move types generated:**
 * - Single pawn pushes (1 square forward)
 * - Double pawn pushes (2 squares from starting rank)
 * - Diagonal captures (left and right)
 * - Promotions (4 piece types on 8th rank)
 * - En passant captures (special case)
 */
void generate_pawn_moves_bb(const Position& pos, S_MOVELIST& list);

/**
 * @brief Generate white pawn moves using bitboard operations
 * @param pos The current position with bitboard state
 * @param list Move list to append generated moves to
 * 
 * Specialized function for white pawn move generation using upward shifts
 * and rank masks for optimal performance.
 */
void generate_white_pawn_moves_bb(const Position& pos, S_MOVELIST& list);

/**
 * @brief Generate black pawn moves using bitboard operations
 * @param pos The current position with bitboard state
 * @param list Move list to append generated moves to
 * 
 * Specialized function for black pawn move generation using downward shifts
 * and rank masks for optimal performance.
 */
void generate_black_pawn_moves_bb(const Position& pos, S_MOVELIST& list);

// ============================================================================
// MOVE CONVERSION UTILITIES
// ============================================================================

/**
 * @brief Add promotion moves for pawn reaching 8th rank
 * @param from Source square of promoting pawn
 * @param to Destination square (8th rank)
 * @param list Move list to append to
 * @param capture_flag Whether this is a capture promotion
 * 
 * Adds all 4 promotion variants (Queen, Rook, Bishop, Knight) for a
 * pawn reaching the promotion rank.
 */
/**
 * @brief Adds all four promotion moves for a given pawn move
 * @param from 64-square index of source square
 * @param to 64-square index of destination square  
 * @param list Move list to add moves to
 * @param capture_flag True if promotion is also a capture
 * @param pos Position to determine captured piece type
 */
void add_promotion_moves(int from, int to, S_MOVELIST& list, bool capture_flag, const Position& pos);

/**
 * @brief Generate en passant captures using bitboard operations
 * @param pos The current position with en passant state
 * @param list Move list to append generated moves to
 * 
 * Handles the special case of en passant captures using bitboard
 * operations for consistency with the Phase 2 migration pattern.
 */
void generate_en_passant_bb(const Position& pos, S_MOVELIST& list);

// ============================================================================
// PERFORMANCE COMPARISON UTILITIES
// ============================================================================

/**
 * @brief Compare old vs new pawn move generation performance
 * @param pos Test position for comparison
 * @param iterations Number of iterations for timing
 * @return Performance ratio (new_time / old_time, <1.0 means improvement)
 * 
 * Benchmarking utility to measure the Phase 2 performance improvement
 * and validate the 15-25% target.
 */
double benchmark_pawn_move_performance(const Position& pos, int iterations = 10000);