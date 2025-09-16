/**
 * @file bitboard_movegen_optimized.hpp
 * @brief High-performance bitboard move generation with minimal overhead
 * 
 * This optimized version addresses the performance bottlenecks in the current
 * bitboard implementation:
 * 
 * 1. Eliminates coordinate conversion overhead
 * 2. Uses more efficient bitboard iteration 
 * 3. Reduces memory access patterns
 * 4. Leverages compiler optimizations
 * 
 * Expected improvements: 30-50% faster than current bitboard implementation
 */

#pragma once

#include "position.hpp"
#include "move.hpp"
#include "bitboard.hpp"
#include "knight_lookup_tables.hpp"
#include "king_lookup_tables.hpp"

namespace OptimizedBitboardMoveGen {

/**
 * @brief Ultra-fast bitboard move generation with minimal overhead
 */
void generate_all_moves_optimized(const Position& pos, S_MOVELIST& list);

/**
 * @brief Optimized knight move generation - eliminate coordinate conversion
 */
void generate_knight_moves_fast(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Optimized pawn move generation using bulk operations
 */
void generate_pawn_moves_fast(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Bulk pawn push processing - eliminate per-move overhead
 */
void generate_pawn_pushes_bulk(uint64_t pushes, S_MOVELIST& list, int offset);

/**
 * @brief Bulk pawn promotion processing
 */
void generate_pawn_promotions_bulk(uint64_t promotions, S_MOVELIST& list, int offset);

/**
 * @brief Fast pawn capture generation
 */
void generate_pawn_captures_fast(const Position& pos, uint64_t pawns, uint64_t enemies, S_MOVELIST& list, Color us);

/**
 * @brief Optimized king move generation
 */
void generate_king_moves_fast(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Fast piece type lookup using bitboard scanning
 */
PieceType get_piece_type_fast(const Position& pos, int square64, Color color);

/**
 * @brief Optimized en passant capture generation
 */
void generate_en_passant_fast(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Main entry point for optimized move generation
 */
void generate_all_moves_optimized(const Position& pos, S_MOVELIST& move_list);

/**
 * @brief Optimized sliding moves generation
 */
void generate_sliding_moves_bulk(const Position& pos, S_MOVELIST& move_list, 
                                U64 pieces, int piece_type, Color color);

/**
 * @brief Optimized knight moves generation
 */
void generate_knight_moves_optimized(const Position& pos, S_MOVELIST& move_list, Color color);

/**
 * @brief Optimized king moves generation
 */
void generate_king_moves_optimized(const Position& pos, S_MOVELIST& move_list, Color color);

/**
 * @brief Optimized pawn moves generation
 */
void generate_pawn_moves_bulk(const Position& pos, S_MOVELIST& move_list, Color color);

/**
 * @brief Optimized sliding piece generation using magic bitboards
 */
template<PieceType PT>
void generate_sliding_moves_fast(const Position& pos, S_MOVELIST& list, Color us);

} // namespace OptimizedBitboardMoveGen