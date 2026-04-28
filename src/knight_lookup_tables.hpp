/**
 * @file knight_lookup_tables.hpp
 * @brief Knight move lookup tables for optimized move generation
 * @author Huginn Chess Engine Development Team
 * @date September 2025
 * @version 1.0
 * 
 * This file implements pre-computed lookup tables for knight moves to optimize
 * the 15.5% of move generation time consumed by knight moves in the Huginn chess engine.
 * 
 * The implementation provides three different approaches:
 * - Array-based lookup tables (fastest)
 * - Bitboard-based lookup
 * - Hybrid template fallback
 * 
 * @performance
 * - Expected improvement: 1-5% over template approach
 * - Memory usage: 2.8KB total (2.3KB arrays + 512 bytes bitboards)
 * - Cache lines: 44 (64-byte lines)
 * 
 * @compatibility
 * - MSVC: Uses _BitScanForward64 and __popcnt64 intrinsics
 * - GCC/Clang: Uses __builtin_ctzll and __builtin_popcountll
 * - Other compilers: Falls back to template method
 */

#pragma once

#include "position.hpp"
#include "move.hpp"
#include "movegen.hpp"
#include "board120.hpp"
#include "chess_types.hpp"
#include "msvc_intrinsics.hpp"
#include <cstdint>

/**
 * @namespace KnightLookupTables
 * @brief Namespace containing knight move optimization lookup tables and functions
 * 
 * This namespace implements pre-computed lookup tables for knight moves to optimize
 * the 15.5% of move generation time consumed by knight moves in the Huginn chess engine.
 * 
 * @section performance Performance Benefits
 * 1. Eliminates repeated boundary checking (IS_PLAYABLE calls)
 * 2. Reduces arithmetic operations per move
 * 3. Better cache locality with 64-entry lookup table
 * 4. Integrates well with bitboard operations if used elsewhere
 * 
 * @section memory Memory Usage
 * - Array tables: 2,304 bytes (64 squares × 9 moves × 4 bytes)
 * - Bitboard table: 512 bytes (64 squares × 8 bytes)
 * - Total: 2,816 bytes (44 cache lines)
 * 
 * @section improvement Expected Improvement
 * 1-5% performance improvement over template approach in typical positions
 */
namespace KnightLookupTables {

    /// @brief Pre-computed knight attack patterns for each square (0-63 for 64-square board)
    /// @details Each entry contains the valid destination squares for a knight on that square
    extern int KNIGHT_MOVES[64][8];  // Max 8 moves per knight
    
    /// @brief Number of valid moves from each square
    /// @details Contains count of legal knight moves for quick iteration bounds
    extern int KNIGHT_MOVE_COUNT[64]; 
    
    /// @brief Bitboard representation of knight attacks for each square
    /// @details Alternative representation using bitboards for engines that support them
    extern uint64_t KNIGHT_ATTACKS[64];
    
    /**
     * @brief Initialize lookup tables - call once at engine startup
     * @details Populates all lookup tables with pre-computed knight move data.
     *          Must be called before using any other functions in this namespace.
     * @post All lookup tables are populated and ready for use
     * @see generate_knight_moves_lookup()
     * @see generate_knight_moves_bitboard()
     */
    void initialize_knight_tables();
    
    /**
     * @brief Debug function to print knight lookup tables (development/debugging only)
     * @details Prints human-readable representation of all lookup tables for verification.
     *          Shows both array-based moves and bitboard representation.
     * 
     * @note This function is intended for development and debugging purposes only.
     *       Not typically called in production builds.
     * 
     * @output Prints formatted tables to stdout showing:
     *         - Square name and index
     *         - Number of available moves  
     *         - List of destination squares in algebraic notation
     *         - Bitboard verification data
     */
    void print_knight_tables();

} // namespace KnightLookupTables