#pragma once

#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "board120.hpp"
#include "chess_types.hpp"
#include <cstdint>

#ifdef _MSC_VER
    #include <intrin.h>
    // MSVC intrinsics for bit operations
    #pragma intrinsic(_BitScanForward64)
    inline int msvc_ctz64(uint64_t value) {
        unsigned long index;
        return _BitScanForward64(&index, value) ? index : 64;
    }
    inline int msvc_popcount64(uint64_t value) {
        return __popcnt64(value);
    }
    #define builtin_ctzll msvc_ctz64
    #define builtin_popcountll msvc_popcount64
#else
    // GCC/Clang built-ins
    #define builtin_ctzll __builtin_ctzll
    #define builtin_popcountll __builtin_popcountll
#endif

/**
 * Huginn Chess Engine - Knight Move Lookup Tables
 * ===============================================
 * 
 * This module implements pre-computed lookup tables for knight moves
 * to further optimize the 15.5% of move generation time consumed by knight moves.
 * 
 * Performance Benefits:
 * 1. Eliminates repeated boundary checking (IS_PLAYABLE calls)
 * 2. Reduces arithmetic operations per move
 * 3. Better cache locality with 64-entry lookup table
 * 4. Integrates well with bitboard operations if used elsewhere
 * 
 * Expected improvement: 5-15% over current template approach
 */

namespace KnightLookupTables {

    // Pre-computed knight attack patterns for each square (0-63 for 64-square board)
    // Each entry contains the valid destination squares for a knight on that square
    extern int KNIGHT_MOVES[64][8];  // Max 8 moves per knight
    extern int KNIGHT_MOVE_COUNT[64]; // Number of valid moves from each square
    
    // Alternative: Bitboard representation (if your engine supports bitboards)
    extern uint64_t KNIGHT_ATTACKS[64];
    
    /**
     * Initialize lookup tables - call once at engine startup
     */
    void initialize_knight_tables();
    
    /**
     * Lookup table-based knight move generation
     * Uses pre-computed tables to eliminate boundary checking and arithmetic
     */
    inline void generate_knight_moves_lookup(const Position& pos, S_MOVELIST& list, Color us) {
        int piece_count = pos.pCount[int(us)][int(PieceType::Knight)];
        
        // Early exit if no knights
        if (piece_count == 0) return;
        
        const auto& knight_list = pos.pList[int(us)][int(PieceType::Knight)];
        
        for (int i = 0; i < piece_count; ++i) {
            int from = knight_list[i];
            if (from == -1) continue;
            
            // Convert from 120-square to 64-square index
            int from_64 = MAILBOX_MAPS.to64[from];
            if (from_64 == -1) continue; // Invalid square
            
            // Use lookup table - no boundary checking needed
            int move_count = KNIGHT_MOVE_COUNT[from_64];
            const int* moves = KNIGHT_MOVES[from_64];
            
            for (int j = 0; j < move_count; ++j) {
                int to_64 = moves[j];
                int to = MAILBOX_MAPS.to120[to_64]; // Convert back to 120-square system
                
                Piece target = pos.at(to);
                if (target == Piece::None) {
                    list.add_quiet_move(make_move(from, to));
                } else if (color_of(target) == !us) {
                    list.add_capture_move(make_capture(from, to, type_of(target)), pos);
                }
            }
        }
    }
    
    /**
     * Bitboard-based knight move generation (if bitboards are available)
     * Most efficient for engines that use bitboard representation
     */
    inline void generate_knight_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us) {
        int piece_count = pos.pCount[int(us)][int(PieceType::Knight)];
        
        if (piece_count == 0) return;
        
        const auto& knight_list = pos.pList[int(us)][int(PieceType::Knight)];
        
        for (int i = 0; i < piece_count; ++i) {
            int from = knight_list[i];
            if (from == -1) continue;
            
            int from_64 = MAILBOX_MAPS.to64[from];
            if (from_64 == -1) continue;
            
            // Get pre-computed attack bitboard
            uint64_t attacks = KNIGHT_ATTACKS[from_64];
            
            // Iterate through attacked squares
            while (attacks) {
                int to_64 = builtin_ctzll(attacks); // Count trailing zeros (LSB)
                attacks &= attacks - 1; // Clear LSB
                
                int to = MAILBOX_MAPS.to120[to_64];
                
                Piece target = pos.at(to);
                if (target == Piece::None) {
                    list.add_quiet_move(make_move(from, to));
                } else if (color_of(target) == !us) {
                    list.add_capture_move(make_capture(from, to, type_of(target)), pos);
                }
            }
        }
    }
    
    /**
     * Hybrid approach: Use lookup for move generation, template for validation
     * Combines the benefits of lookup tables with compile-time optimization
     */
    template<bool UseLookup = true>
    inline void generate_knight_moves_hybrid(const Position& pos, S_MOVELIST& list, Color us) {
        if constexpr (UseLookup) {
            generate_knight_moves_lookup(pos, list, us);
        } else {
            // Fallback to template approach for comparison
            // Note: KnightOptimizations included at use site to avoid circular dependency
            // Include "knight_optimizations.hpp" in the calling file
        }
    }

} // namespace KnightLookupTables