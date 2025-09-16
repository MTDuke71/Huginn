/**
 * @file bitboard_movegen_pure.hpp
 * @brief Native bitboard move generation
 * 
 * Pure 64-square bitboard move generation that eliminates all conversion overhead.
 * Designed for maximum performance by working directly with bitboards without
 * any mailbox dependencies.
 * 
 * @author MTDuke71
 * @version 1.0
 */
#pragma once

#include "bitboard_position.hpp"
#include "move.hpp"
#include <vector>

// ============================================================================
// BITBOARD MOVE LIST
// ============================================================================

/**
 * @brief Move list optimized for bitboard operations
 * 
 * Simplified move list that stores moves in 64-square format.
 * Can be converted to S_MOVELIST when interfacing with 120-square code.
 */
struct BitboardMoveList {
    struct BitboardMove {
        int from_64;
        int to_64;
        PieceType promotion_type;
        bool is_capture;
        bool is_ep_capture;
        bool is_castling;
        bool is_promotion;
        
        BitboardMove(int f, int t, PieceType promo = PieceType::None) 
            : from_64(f), to_64(t), promotion_type(promo), is_capture(false), 
              is_ep_capture(false), is_castling(false), is_promotion(promo != PieceType::None) {}
    };
    
    std::vector<BitboardMove> moves;
    
    void clear() { moves.clear(); }
    void add_move(int from_64, int to_64, PieceType promo = PieceType::None) {
        moves.emplace_back(from_64, to_64, promo);
    }
    void add_capture(int from_64, int to_64, PieceType promo = PieceType::None) {
        BitboardMove move(from_64, to_64, promo);
        move.is_capture = true;
        moves.push_back(move);
    }
    size_t size() const { return moves.size(); }
    const BitboardMove& operator[](size_t i) const { return moves[i]; }
};

// ============================================================================
// NATIVE BITBOARD MOVE GENERATION
// ============================================================================

namespace BitboardMoveGen {

/**
 * @brief Generate all legal moves using pure bitboard operations
 * 
 * Main entry point for native bitboard move generation. Eliminates all
 * conversion overhead by working directly in 64-square space.
 */
void generate_all_moves(const BitboardPosition& pos, BitboardMoveList& moves);

/**
 * @brief Generate only legal moves by filtering out illegal moves
 * 
 * First generates all pseudo-legal moves, then filters out moves that would
 * leave the king in check (including pinned pieces and discovered checks).
 */
void generate_legal_moves(BitboardPosition& pos, BitboardMoveList& moves);

/**
 * @brief Generate pawn moves using optimized bitboard operations
 * 
 * Pure bitboard pawn move generation with no conversion overhead.
 * Handles single pushes, double pushes, captures, en passant, and promotions.
 */
void generate_pawn_moves(const BitboardPosition& pos, BitboardMoveList& moves);

/**
 * @brief Generate knight moves using bitboard operations
 */
void generate_knight_moves(const BitboardPosition& pos, BitboardMoveList& moves);

/**
 * @brief Generate bishop moves using bitboard operations
 * 
 * Uses magic bitboard attack generation for maximum performance.
 * Processes all bishops with bulk bitboard operations.
 */
void generate_bishop_moves(const BitboardPosition& pos, BitboardMoveList& moves);

/**
 * @brief Generate rook moves using bitboard operations
 * 
 * Uses magic bitboard attack generation for maximum performance.
 * Processes all rooks with bulk bitboard operations.
 */
void generate_rook_moves(const BitboardPosition& pos, BitboardMoveList& moves);

/**
 * @brief Generate queen moves using bitboard operations
 * 
 * Combines bishop and rook attacks for complete queen mobility.
 * Processes all queens with bulk bitboard operations.
 */
void generate_queen_moves(const BitboardPosition& pos, BitboardMoveList& moves);/**
 * @brief Generate king moves using bitboard operations
 */
void generate_king_moves(const BitboardPosition& pos, BitboardMoveList& moves);

// ============================================================================
// OPTIMIZED PAWN MOVE GENERATION
// ============================================================================

/**
 * @brief Process white pawn moves with pure bitboard operations
 * 
 * Optimized implementation that eliminates conversion overhead by
 * processing entire bitboards without individual square operations.
 */
void generate_white_pawn_moves_optimized(const BitboardPosition& pos, BitboardMoveList& moves);

/**
 * @brief Process black pawn moves with pure bitboard operations
 */
void generate_black_pawn_moves_optimized(const BitboardPosition& pos, BitboardMoveList& moves);

// ============================================================================
// BITBOARD MOVE UTILITIES
// ============================================================================

/**
 * @brief Add moves from a bitboard to the move list
 * 
 * Efficiently extracts all set bits from a bitboard and adds them as moves.
 * Eliminates individual conversion operations.
 */
void add_moves_from_bitboard(uint64_t move_bitboard, int from_square, 
                            BitboardMoveList& moves, bool is_capture = false);

/**
 * @brief Add promotion moves from a bitboard
 */
void add_promotion_moves_from_bitboard(uint64_t promo_bitboard, int from_square,
                                      BitboardMoveList& moves, bool is_capture = false);

/**
 * @brief Generate sliding piece moves (bishop/rook/queen)
 * 
 * Generic sliding piece move generation using attack bitboards.
 */
uint64_t generate_sliding_attacks(int square_64, uint64_t occupied, 
                                 const uint64_t* direction_masks);

} // namespace BitboardMoveGen