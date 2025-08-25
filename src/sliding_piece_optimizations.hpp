#pragma once

#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "board120.hpp"
#include "chess_types.hpp"

/**
 * Huginn Chess Engine - Sliding Piece Move Generation Optimizations
 * =================================================================
 * 
 * This module implements optimized sliding piece move generation targeting
 * the combined 45%+ of move generation time consumed by Bishop/Rook/Queen moves.
 * 
 * Current Performance (from profiling):
 * - Bishop Moves: 15.6% of time (6.6 ns/move)
 * - Rook Moves: 13.4% of time (11.6 ns/move) 
 * - Queen Moves: 14.8% of time (7.5 ns/move)
 * 
 * Optimization Strategy:
 * 1. Template-based direction unrolling for compile-time optimization
 * 2. Specialized functions for each piece type (eliminate generic overhead)
 * 3. Early exit optimizations for positions with no pieces
 * 4. Improved memory access patterns and instruction pipelining
 */

namespace SlidingPieceOptimizations {

    /**
     * Template-based sliding move generation for specific directions
     * Allows compiler to fully optimize each direction at compile-time
     */
    template<int Direction>
    inline void generate_sliding_ray(const Position& pos, S_MOVELIST& list, Color us, int from) {
        for (int to = from + Direction; IS_PLAYABLE(to); to += Direction) {
            Piece target = pos.at(to);
            
            if (target == Piece::None) {
                list.add_quiet_move(make_move(from, to));
            } else {
                if (color_of(target) == !us) {
                    list.add_capture_move(make_capture(from, to, type_of(target)), pos);
                }
                break;  // Piece blocks further movement
            }
        }
    }

    /**
     * Optimized bishop move generation with unrolled directions
     * Eliminates direction loop overhead for better performance
     */
    inline void generate_bishop_moves_optimized(const Position& pos, S_MOVELIST& list, Color us) {
        int piece_count = pos.pCount[int(us)][int(PieceType::Bishop)];
        
        // Early exit if no bishops
        if (piece_count == 0) return;
        
        const auto& bishop_list = pos.pList[int(us)][int(PieceType::Bishop)];
        
        for (int i = 0; i < piece_count; ++i) {
            int from = bishop_list[i];
            if (from == -1) continue;
            
            // Unrolled diagonal directions for bishops
            generate_sliding_ray<NE>(pos, list, us, from);   // Northeast
            generate_sliding_ray<NW>(pos, list, us, from);   // Northwest  
            generate_sliding_ray<SE>(pos, list, us, from);   // Southeast
            generate_sliding_ray<SW>(pos, list, us, from);   // Southwest
        }
    }

    /**
     * Optimized rook move generation with unrolled directions
     */
    inline void generate_rook_moves_optimized(const Position& pos, S_MOVELIST& list, Color us) {
        int piece_count = pos.pCount[int(us)][int(PieceType::Rook)];
        
        // Early exit if no rooks
        if (piece_count == 0) return;
        
        const auto& rook_list = pos.pList[int(us)][int(PieceType::Rook)];
        
        for (int i = 0; i < piece_count; ++i) {
            int from = rook_list[i];
            if (from == -1) continue;
            
            // Unrolled orthogonal directions for rooks
            generate_sliding_ray<NORTH>(pos, list, us, from);  // North
            generate_sliding_ray<SOUTH>(pos, list, us, from);  // South
            generate_sliding_ray<EAST>(pos, list, us, from);   // East
            generate_sliding_ray<WEST>(pos, list, us, from);   // West
        }
    }

    /**
     * Optimized queen move generation with unrolled directions
     * Combines both diagonal and orthogonal rays
     */
    inline void generate_queen_moves_optimized(const Position& pos, S_MOVELIST& list, Color us) {
        int piece_count = pos.pCount[int(us)][int(PieceType::Queen)];
        
        // Early exit if no queens
        if (piece_count == 0) return;
        
        const auto& queen_list = pos.pList[int(us)][int(PieceType::Queen)];
        
        for (int i = 0; i < piece_count; ++i) {
            int from = queen_list[i];
            if (from == -1) continue;
            
            // Unrolled all eight directions for queens
            // Orthogonal directions (rook-like)
            generate_sliding_ray<NORTH>(pos, list, us, from);
            generate_sliding_ray<SOUTH>(pos, list, us, from);
            generate_sliding_ray<EAST>(pos, list, us, from);
            generate_sliding_ray<WEST>(pos, list, us, from);
            
            // Diagonal directions (bishop-like)
            generate_sliding_ray<NE>(pos, list, us, from);
            generate_sliding_ray<NW>(pos, list, us, from);
            generate_sliding_ray<SE>(pos, list, us, from);
            generate_sliding_ray<SW>(pos, list, us, from);
        }
    }

    /**
     * Alternative implementation using macro unrolling
     * Provides balance between performance and code maintainability
     */
    #define GENERATE_SLIDING_RAY(direction) \
        for (int to = from + (direction); IS_PLAYABLE(to); to += (direction)) { \
            Piece target = pos.at(to); \
            if (target == Piece::None) { \
                list.add_quiet_move(make_move(from, to)); \
            } else { \
                if (color_of(target) == !us) { \
                    list.add_capture_move(make_capture(from, to, type_of(target)), pos); \
                } \
                break; \
            } \
        }

    inline void generate_bishop_moves_macro(const Position& pos, S_MOVELIST& list, Color us) {
        int piece_count = pos.pCount[int(us)][int(PieceType::Bishop)];
        if (piece_count == 0) return;
        
        const auto& bishop_list = pos.pList[int(us)][int(PieceType::Bishop)];
        
        for (int i = 0; i < piece_count; ++i) {
            int from = bishop_list[i];
            if (from == -1) continue;
            
            GENERATE_SLIDING_RAY(NE);
            GENERATE_SLIDING_RAY(NW);
            GENERATE_SLIDING_RAY(SE);
            GENERATE_SLIDING_RAY(SW);
        }
    }

    inline void generate_rook_moves_macro(const Position& pos, S_MOVELIST& list, Color us) {
        int piece_count = pos.pCount[int(us)][int(PieceType::Rook)];
        if (piece_count == 0) return;
        
        const auto& rook_list = pos.pList[int(us)][int(PieceType::Rook)];
        
        for (int i = 0; i < piece_count; ++i) {
            int from = rook_list[i];
            if (from == -1) continue;
            
            GENERATE_SLIDING_RAY(NORTH);
            GENERATE_SLIDING_RAY(SOUTH);
            GENERATE_SLIDING_RAY(EAST);
            GENERATE_SLIDING_RAY(WEST);
        }
    }

    inline void generate_queen_moves_macro(const Position& pos, S_MOVELIST& list, Color us) {
        int piece_count = pos.pCount[int(us)][int(PieceType::Queen)];
        if (piece_count == 0) return;
        
        const auto& queen_list = pos.pList[int(us)][int(PieceType::Queen)];
        
        for (int i = 0; i < piece_count; ++i) {
            int from = queen_list[i];
            if (from == -1) continue;
            
            // All eight directions
            GENERATE_SLIDING_RAY(NORTH);
            GENERATE_SLIDING_RAY(SOUTH);
            GENERATE_SLIDING_RAY(EAST);
            GENERATE_SLIDING_RAY(WEST);
            GENERATE_SLIDING_RAY(NE);
            GENERATE_SLIDING_RAY(NW);
            GENERATE_SLIDING_RAY(SE);
            GENERATE_SLIDING_RAY(SW);
        }
    }

    #undef GENERATE_SLIDING_RAY

    /**
     * Combined sliding piece generation that can handle multiple piece types
     * in a single optimized function to reduce function call overhead
     */
    inline void generate_all_sliding_moves_optimized(const Position& pos, S_MOVELIST& list, Color us) {
        // Process bishops
        generate_bishop_moves_optimized(pos, list, us);
        
        // Process rooks
        generate_rook_moves_optimized(pos, list, us);
        
        // Process queens
        generate_queen_moves_optimized(pos, list, us);
    }

    /**
     * Optimized sliding piece generation with better instruction ordering
     * Groups similar operations together for better CPU pipeline utilization
     */
    inline void generate_sliding_moves_pipeline_optimized(const Position& pos, S_MOVELIST& list, Color us) {
        // Check piece counts first (better branch prediction)
        int bishop_count = pos.pCount[int(us)][int(PieceType::Bishop)];
        int rook_count = pos.pCount[int(us)][int(PieceType::Rook)];
        int queen_count = pos.pCount[int(us)][int(PieceType::Queen)];
        
        // Early exit if no sliding pieces at all
        if (bishop_count == 0 && rook_count == 0 && queen_count == 0) return;
        
        // Process pieces in order of typical frequency (rook, bishop, queen)
        if (rook_count > 0) {
            generate_rook_moves_optimized(pos, list, us);
        }
        
        if (bishop_count > 0) {
            generate_bishop_moves_optimized(pos, list, us);
        }
        
        if (queen_count > 0) {
            generate_queen_moves_optimized(pos, list, us);
        }
    }

} // namespace SlidingPieceOptimizations
