/**
 * @file bitboard_perft_optimized.hpp
 * @brief Ultra-fast legal move generation for BitboardPosition perft
 * 
 * This module eliminates the major performance bottleneck in perft testing:
 * the expensive make/unmake cycle for every pseudo-legal move just to test legality.
 * 
 * Key Performance Improvements:
 * 1. Direct legal move generation (no make/unmake overhead)
 * 2. Efficient in-check detection during generation
 * 3. Fast pin detection using bitboard rays
 * 4. Bulk move processing where possible
 * 
 * Expected Performance Gain: 3-5x faster than current approach
 * 
 * @author MTDuke71 
 * @version 1.0
 */

#pragma once

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"

namespace BitboardPerftOptimized {

// ============================================================================
// MAIN OPTIMIZED ENTRY POINT
// ============================================================================

/**
 * @brief Ultra-fast legal move generation for perft
 * 
 * Generates legal moves directly without expensive make/unmake testing.
 * Uses efficient check detection and pin analysis.
 */
void generate_legal_moves_fast(const BitboardPosition& pos, BitboardMoveList& moves);

// ============================================================================
// EFFICIENT ATTACK DETECTION
// ============================================================================

/**
 * @brief Fast square attack detection using precomputed tables
 * 
 * Much faster than the current make/unmake approach for legality testing.
 */
bool is_square_attacked_fast(const BitboardPosition& pos, int square, Color attacking_color);

/**
 * @brief Check if square would be attacked with different occupancy
 * 
 * Used for king move legality without actually moving the king.
 */
bool is_square_attacked_with_occupied(const BitboardPosition& pos, int square, 
                                     Color attacking_color, uint64_t occupied);

// ============================================================================
// PIN AND CHECK DETECTION
// ============================================================================

/**
 * @brief Find all pieces pinned to the king
 * 
 * Uses bitboard ray intersections to efficiently detect pins.
 */
uint64_t find_pinned_pieces(const BitboardPosition& pos, int king_square);

/**
 * @brief Find all pieces attacking the king
 * 
 * Efficient check detection for evasion generation.
 */
void find_attackers(const BitboardPosition& pos, int king_square, 
                   Color attacking_color, std::vector<int>& attackers);

/**
 * @brief Find pin ray between king and attacker
 */
uint64_t find_pin_ray(const BitboardPosition& pos, int king_square, int attacker_square, 
                     uint64_t occupied, bool is_diagonal);

// ============================================================================
// SPECIALIZED MOVE GENERATION
// ============================================================================

/**
 * @brief Generate check evasion moves only
 * 
 * Used when king is in check - much more efficient than testing all moves.
 */
void generate_check_evasions(const BitboardPosition& pos, BitboardMoveList& moves, int king_square);

/**
 * @brief Generate king moves that don't walk into check
 */
void generate_king_evasions(const BitboardPosition& pos, BitboardMoveList& moves, int king_square);

/**
 * @brief Generate blocks and captures to escape check
 */
void generate_blocks_and_captures(const BitboardPosition& pos, BitboardMoveList& moves, 
                                 int king_square, int attacker_square);

/**
 * @brief Generate all legal moves when not in check
 */
void generate_all_legal_moves_normal(const BitboardPosition& pos, BitboardMoveList& moves, int king_square);

// ============================================================================
// OPTIMIZED PIECE MOVE GENERATION
// ============================================================================

/**
 * @brief Generate legal king moves efficiently
 */
void generate_king_moves_legal(const BitboardPosition& pos, BitboardMoveList& moves, int king_square);

/**
 * @brief Generate legal pawn moves with pin awareness
 */
void generate_pawn_moves_legal(const BitboardPosition& pos, BitboardMoveList& moves, int king_square);
void generate_pawn_moves_legal(const BitboardPosition& pos, BitboardMoveList& moves, int king_square, uint64_t pinned_pieces);

// Pin detection
uint64_t find_pinned_pieces(const BitboardPosition& pos, int king_square);

/**
 * @brief Generate moves for pinned pawns (restricted to pin ray)
 */
void generate_pawn_moves_pinned(const BitboardPosition& pos, BitboardMoveList& moves, 
                               uint64_t pinned_pawns, int king_square);

/**
 * @brief Bulk pawn move generation for unpinned pawns
 */
void generate_pawn_moves_bulk(const BitboardPosition& pos, BitboardMoveList& moves, 
                             uint64_t pawns, bool pinned);

/**
 * @brief Generate legal knight moves with pin checking
 */
void generate_knight_moves_legal(const BitboardPosition& pos, BitboardMoveList& moves, int king_square);

/**
 * @brief Generate legal bishop moves with pin checking  
 */
void generate_bishop_moves_legal(const BitboardPosition& pos, BitboardMoveList& moves, int king_square);

/**
 * @brief Generate legal rook moves with pin checking
 */
void generate_rook_moves_legal(const BitboardPosition& pos, BitboardMoveList& moves, int king_square);

/**
 * @brief Generate legal queen moves with pin checking
 */
void generate_queen_moves_legal(const BitboardPosition& pos, BitboardMoveList& moves, int king_square);

/**
 * @brief Generate legal castling moves
 */
void generate_castling_moves_legal(const BitboardPosition& pos, BitboardMoveList& moves, int king_square);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Check if king move would be safe
 */
bool would_be_attacked_after_king_move(const BitboardPosition& pos, int to_square, 
                                      Color enemy_color, int king_square);

/**
 * @brief Get ray between two squares for pin detection
 */
uint64_t get_ray_between(int from_square, int to_square);

/**
 * @brief Simplified sliding piece attack detection for Phase 1
 */
bool is_attacked_by_sliding_piece_simple(int square, uint64_t attackers, uint64_t occupied, bool diagonal);

/**
 * @brief Check if square is on diagonal ray from attacker
 */
bool is_on_diagonal_ray(int square, int attacker_square, uint64_t occupied);

/**
 * @brief Check if square is on straight ray from attacker  
 */
bool is_on_straight_ray(int square, int attacker_square, uint64_t occupied);

/**
 * @brief Check if a pawn move is legal (doesn't leave king in check)
 */
bool is_pawn_move_legal(const BitboardPosition& pos, int from_square, int to_square, bool is_capture, int king_square);

/**
 * @brief Check if a knight move is legal (doesn't leave king in check)
 */
bool is_knight_move_legal(const BitboardPosition& pos, int from_square, int to_square, int king_square);

/**
 * @brief Check if a sliding piece move is legal (doesn't leave king in check)
 */
bool is_sliding_move_legal(const BitboardPosition& pos, int from_square, int to_square, int king_square);

/**
 * @brief Generate diagonal attacks for bishop-like pieces
 */
uint64_t generate_diagonal_attacks(int square, uint64_t occupied);

/**
 * @brief Generate straight attacks for rook-like pieces
 */
uint64_t generate_straight_attacks(int square, uint64_t occupied);

/**
 * @brief Fast pop_lsb that doesn't modify the bitboard
 */
#ifdef _MSC_VER
    #include <intrin.h>
    inline int pop_lsb_copy(uint64_t bb) {
        unsigned long index;
        _BitScanForward64(&index, bb);
        return static_cast<int>(index);
    }
    
    inline int pop_lsb(uint64_t& bb) {
        unsigned long index;
        _BitScanForward64(&index, bb);
        bb &= bb - 1;  // Clear the LSB
        return static_cast<int>(index);
    }
#else
    inline int pop_lsb_copy(uint64_t bb) {
        return __builtin_ctzll(bb);
    }
    
    inline int pop_lsb(uint64_t& bb) {
        int pos = __builtin_ctzll(bb);
        bb &= bb - 1;  // Clear the LSB
        return pos;
    }
#endif

} // namespace BitboardPerftOptimized