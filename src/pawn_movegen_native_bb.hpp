/**
 * @file pawn_movegen_native_bb.hpp
 * @brief Native bitboard pawn move generation without conversion overhead
 * 
 * This implementation eliminates the performance bottleneck from the previous
 * approach by avoiding bitboard-to-mailbox conversion during move generation.
 * Instead, it processes entire bitboards in bulk and converts only at the
 * final integration point.
 * 
 * ## Performance Bottleneck Analysis
 * 
 * **Previous Implementation (2.3% improvement):**
 * ```cpp
 * while (bitboard != 0) {
 *     int to = pop_lsb(bitboard);           // Extract individual squares
 *     int from = to - offset;               // Calculate source
 *     int from_120 = MAILBOX_MAPS.to120[from]; // CONVERSION OVERHEAD
 *     int to_120 = MAILBOX_MAPS.to120[to];     // CONVERSION OVERHEAD  
 *     list.add_move(make_move(from_120, to_120)); // Individual move creation
 * }
 * ```
 * 
 * **New Native Approach (Target: 15-25% improvement):**
 * ```cpp
 * // Process entire bitboards without conversion
 * BulkMoveData bulk_data = process_pawn_bitboards_bulk(pos);
 * 
 * // Convert only once at integration point
 * integrate_bulk_moves_to_list(bulk_data, list);
 * ```
 * 
 * ## Optimization Strategy
 * 
 * 1. **Bulk Bitboard Processing**: Handle entire bitboards at once
 * 2. **Deferred Conversion**: Convert coordinates only when integrating with move list
 * 3. **SIMD-Friendly Operations**: Use count/offset operations instead of individual extraction
 * 4. **Vectorized Move Creation**: Create multiple moves simultaneously
 * 
 * @author MTDuke71
 * @version 2.0 - Native Bitboard Implementation
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
// BULK MOVE DATA STRUCTURES
// ============================================================================

/**
 * @brief Container for bulk-processed pawn moves before conversion
 * 
 * This structure holds bitboard move data in native format,
 * avoiding individual square conversions until the final integration step.
 */
struct BulkPawnMoves {
    // Regular moves (no conversion overhead during generation)
    uint64_t single_pushes = 0;
    uint64_t double_pushes = 0; 
    uint64_t left_captures = 0;
    uint64_t right_captures = 0;
    
    // Promotion moves (8th rank)
    uint64_t promotion_pushes = 0;
    uint64_t promotion_left_captures = 0;
    uint64_t promotion_right_captures = 0;
    
    // En passant targets
    uint64_t en_passant_targets = 0;
    
    // Move offsets for bulk conversion
    static constexpr int SINGLE_PUSH_OFFSET = 8;
    static constexpr int DOUBLE_PUSH_OFFSET = 16;
    static constexpr int LEFT_CAPTURE_OFFSET = 7;
    static constexpr int RIGHT_CAPTURE_OFFSET = 9;
};

// ============================================================================
// NATIVE BITBOARD MOVE GENERATION
// ============================================================================

/**
 * @brief Generate pawn moves using native bitboard processing (Phase 2 Optimized)
 * @param pos The current position with bitboard state
 * @param list Move list to append generated moves to
 * 
 * This function implements the optimized approach that eliminates conversion
 * overhead by processing entire bitboards before any mailbox conversion.
 * 
 * **Performance target**: 15-25% improvement over original piece-list approach
 */
void generate_pawn_moves_native_bb(const Position& pos, S_MOVELIST& list);

/**
 * @brief Process white pawn bitboards in bulk without conversion overhead
 * @param pos Position containing bitboard state
 * @return BulkPawnMoves containing all white pawn move bitboards
 */
BulkPawnMoves process_white_pawn_bitboards_bulk(const Position& pos);

/**
 * @brief Process black pawn bitboards in bulk without conversion overhead  
 * @param pos Position containing bitboard state
 * @return BulkPawnMoves containing all black pawn move bitboards
 */
BulkPawnMoves process_black_pawn_bitboards_bulk(const Position& pos);

/**
 * @brief Integrate bulk move data into S_MOVELIST with minimal conversion
 * @param bulk_moves Pre-processed bitboard move data
 * @param list Target move list for integration
 * @param us Color of the moves being integrated
 * @param pos Position for move validation and captured piece detection
 * 
 * This function performs the minimal necessary conversion from bitboard
 * coordinates to the engine's move format, optimized for bulk processing.
 */
void integrate_bulk_pawn_moves(const BulkPawnMoves& bulk_moves, S_MOVELIST& list, 
                               Color us, const Position& pos);

/**
 * @brief Process en passant moves with native bitboard operations
 * @param pos Position containing en passant state
 * @param list Move list to append en passant moves to
 */
void generate_en_passant_native_bb(const Position& pos, S_MOVELIST& list);

// ============================================================================
// BULK CONVERSION UTILITIES  
// ============================================================================

/**
 * @brief Add bulk promotion moves for bitboard of promoting pawns
 * @param promotion_bitboard Bitboard containing promotion destination squares
 * @param list Target move list  
 * @param offset Source square offset
 * @param is_capture Whether these are capture promotions
 * @param pos Position for captured piece detection
 */
void bulk_add_promotion_moves(uint64_t promotion_bitboard, S_MOVELIST& list,
                             int offset, bool is_capture, const Position& pos);

// ============================================================================
// PERFORMANCE COMPARISON
// ============================================================================

/**
 * @brief Benchmark native bitboard implementation vs conversion-heavy approach
 * @param pos Test position
 * @param iterations Number of benchmark iterations
 * @return Performance comparison data
 */
struct NativeBitboardBenchmark {
    double native_time_ms;
    double conversion_time_ms; 
    double improvement_percent;
    int moves_generated;
};

NativeBitboardBenchmark benchmark_native_vs_conversion(const Position& pos, int iterations);