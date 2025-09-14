/**
 * @file knight_lookup_tables.cpp
 * @brief Implementation of knight move lookup tables for optimized move generation
 * @author Huginn Chess Engine Development Team
 * @date September 2025
 * @version 1.0
 * 
 * This file contains the implementation of pre-computed knight move lookup tables
 * for the Huginn chess engine. The tables are generated once at startup and provide
 * fast, boundary-check-free knight move generation.
 */

#include "knight_lookup_tables.hpp"
#include "board120.hpp"
#include "msvc_intrinsics.hpp"

namespace KnightLookupTables {

    /// @brief Pre-computed knight moves for each square (array representation)
    int KNIGHT_MOVES[64][8];
    
    /// @brief Number of valid knight moves from each square
    int KNIGHT_MOVE_COUNT[64];
    
    /// @brief Pre-computed knight attacks for each square (bitboard representation)
    uint64_t KNIGHT_ATTACKS[64];
    
    /**
     * @brief Convert 64-square board index to rank (0-7)
     * @param sq64 Square index in 64-square representation (0-63)
     * @return Rank number (0-7, where 0 is rank 1 and 7 is rank 8)
     */
    inline int square_to_rank(int sq64) { return sq64 / 8; }
    
    /**
     * @brief Convert 64-square board index to file (0-7)  
     * @param sq64 Square index in 64-square representation (0-63)
     * @return File number (0-7, where 0 is file A and 7 is file H)
     */
    inline int square_to_file(int sq64) { return sq64 % 8; }
    
    /**
     * @brief Convert rank and file coordinates to 64-square index
     * @param rank Rank number (0-7)
     * @param file File number (0-7)
     * @return Square index (0-63), or -1 if coordinates are invalid
     */
    inline int rank_file_to_square(int rank, int file) { 
        if (rank < 0 || rank > 7 || file < 0 || file > 7) return -1;
        return rank * 8 + file; 
    }
    
    /**
     * @brief Initialize knight lookup tables at engine startup
     * @details Populates KNIGHT_MOVES, KNIGHT_MOVE_COUNT, and KNIGHT_ATTACKS arrays
     *          with pre-computed knight move data for all 64 squares.
     * 
     * @post All lookup tables are populated:
     *       - KNIGHT_MOVES[sq][i] contains valid destination squares
     *       - KNIGHT_MOVE_COUNT[sq] contains number of moves from square sq
     *       - KNIGHT_ATTACKS[sq] contains bitboard of attacked squares
     * 
     * @complexity O(64 * 8) = O(512) constant time initialization
     * 
     * @threadsafety Not thread-safe. Must be called before any concurrent access.
     */
    void initialize_knight_tables() {
        /// @brief Knight move offsets in rank/file coordinates (all 8 L-shaped moves)
        const int knight_deltas[8][2] = {
            {-2, -1}, {-2, +1}, {-1, -2}, {-1, +2},
            {+1, -2}, {+1, +2}, {+2, -1}, {+2, +1}
        };
        
        // Initialize lookup tables for all 64 squares
        for (int sq64 = 0; sq64 < 64; ++sq64) {
            int rank = square_to_rank(sq64);
            int file = square_to_file(sq64);
            
            int move_count = 0;
            uint64_t attack_bitboard = 0ULL;
            
            // Check all 8 possible knight moves from current square
            for (int i = 0; i < 8; ++i) {
                int new_rank = rank + knight_deltas[i][0];
                int new_file = file + knight_deltas[i][1];
                
                int target_sq = rank_file_to_square(new_rank, new_file);
                
                if (target_sq != -1) { // Valid square (within board boundaries)
                    KNIGHT_MOVES[sq64][move_count] = target_sq;
                    attack_bitboard |= (1ULL << target_sq);
                    move_count++;
                }
            }
            
            // Store computed values in lookup tables
            KNIGHT_MOVE_COUNT[sq64] = move_count;
            KNIGHT_ATTACKS[sq64] = attack_bitboard;
            
            // Fill remaining array slots with -1 for safety and debugging
            for (int i = move_count; i < 8; ++i) {
                KNIGHT_MOVES[sq64][i] = -1;
            }
        }
    }
    
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
    void print_knight_tables() {
        printf("Knight Lookup Tables:\n");
        printf("=====================\n\n");
        
        for (int sq64 = 0; sq64 < 64; ++sq64) {
            int rank = square_to_rank(sq64);
            int file = square_to_file(sq64);
            char square_name[3];
            sprintf(square_name, "%c%d", 'a' + file, rank + 1);
            
            printf("Square %s (%d): %d moves -> ", 
                   square_name, sq64, KNIGHT_MOVE_COUNT[sq64]);
            
            for (int i = 0; i < KNIGHT_MOVE_COUNT[sq64]; ++i) {
                int target = KNIGHT_MOVES[sq64][i];
                int target_rank = square_to_rank(target);
                int target_file = square_to_file(target);
                printf("%c%d ", 'a' + target_file, target_rank + 1);
            }
            printf("\n");
            
            if ((sq64 + 1) % 8 == 0) printf("\n"); // Line break every rank
        }
        
        printf("Bitboard verification:\n");
        printf("======================\n");
        
        // Verify consistency between array and bitboard representations
        for (int sq64 = 0; sq64 < 64; ++sq64) {
            if (KNIGHT_MOVE_COUNT[sq64] != builtin_popcountll(KNIGHT_ATTACKS[sq64])) {
                printf("ERROR: Mismatch at square %d\n", sq64);
            }
        }
        printf("Lookup table initialization complete.\n\n");
    }

} // namespace KnightLookupTables