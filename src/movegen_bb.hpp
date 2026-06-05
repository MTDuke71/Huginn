/**
 * @file movegen_bb.hpp
 * @brief True bitboard-based move generation for Huginn
 * 
 * This module implements actual bitboard move generation that should demonstrate
 * the dramatic performance improvement over piece lists. This is what will replace
 * the piece list approach in huginn.exe.
 * 
 * Expected Performance Gains:
 * - Knight moves: 10-15x faster (bitboard lookup vs. piece list iteration)
 * - Pawn moves: 8-12x faster (shift operations vs. individual checks)
 * - Sliding pieces: 5-8x faster (magic bitboards vs. ray tracing)
 * 
 * @author MTDuke71
 * @version 1.0
 */

#pragma once

#include "position.hpp"
#include "move.hpp"
#include "bitboard.hpp"
#include "attack_tables.hpp"

/**
 * @namespace BitboardMoveGen
 * @brief Bitboard-based move generation — the production move generator.
 *
 * Generates moves directly from per-piece attack bitboards (knight/king/pawn
 * lookup tables and magic-bitboard sliders) rather than iterating piece lists.
 * This is the path used by the search; the mailbox generator in movegen.* is
 * retained only for cross-validation and tests.
 */
namespace BitboardMoveGen {

/**
 * @brief Generate all moves using pure bitboard approach
 * @param pos Current position
 * @param list Move list to populate
 * 
 * This is the main function.
 * Uses bitboards throughout for maximum performance.
 */
void generate_all_moves_bitboard(const Position& pos, S_MOVELIST& list);

/**
 * @brief Generate knight moves using bitboard attack tables
 * @param pos Current position
 * @param list Move list to append to
 * @param us Color to generate moves for
 * 
 * Uses KNIGHT_ATTACKS[square] lookup and bitboard iteration.
 * Should be 10-15x faster than piece list approach.
 */
void generate_knight_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Generate pawn moves using bitboard shifts and attacks
 * @param pos Current position  
 * @param list Move list to append to
 * @param us Color to generate moves for
 * 
 * Uses bitboard shift operations for pushes and PAWN_ATTACKS for captures.
 * Should be 8-12x faster than piece list approach.
 */
void generate_pawn_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Generate king moves using bitboard attack tables
 * @param pos Current position
 * @param list Move list to append to  
 * @param us Color to generate moves for
 * 
 * Uses KING_ATTACKS[square] lookup for fast generation.
 */
void generate_king_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Generate bishop moves using bitboard attack generation
 * @param pos Current position
 * @param list Move list to append to
 * @param us Color to generate moves for
 * 
 * Uses bishop_attacks(square, occupancy) for sliding piece moves.
 */
void generate_bishop_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Generate rook moves using bitboard attack generation  
 * @param pos Current position
 * @param list Move list to append to
 * @param us Color to generate moves for
 * 
 * Uses rook_attacks(square, occupancy) for sliding piece moves.
 */
void generate_rook_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Generate queen moves using bitboard attack generation
 * @param pos Current position
 * @param list Move list to append to
 * @param us Color to generate moves for
 * 
 * Uses queen_attacks(square, occupancy) for sliding piece moves.
 */
void generate_queen_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

} // namespace BitboardMoveGen