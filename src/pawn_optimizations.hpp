#pragma once
#include "movegen_enhanced.hpp"
#include "board120.hpp"

// Pawn Promotion Optimization Module
// Based on profiling results showing pawn moves consume 20.3% of generation time

namespace PawnOptimizations {

// Pre-computed promotion squares for fast lookup
struct PromotionSquares {
    // White promotion squares (rank 8: squares 91-98)
    static constexpr int WHITE_PROMOTION_RANK = 91;
    static constexpr bool is_white_promotion_square(int sq) {
        return sq >= 91 && sq <= 98;
    }
    
    // Black promotion squares (rank 1: squares 21-28) 
    static constexpr int BLACK_PROMOTION_RANK = 21;
    static constexpr bool is_black_promotion_square(int sq) {
        return sq >= 21 && sq <= 28;
    }
    
    // Combined check
    static constexpr bool is_promotion_square(int sq, Color us) {
        return (us == Color::White) ? is_white_promotion_square(sq) : is_black_promotion_square(sq);
    }
};

// Optimized promotion piece generation order (highest value first for better move ordering)
static constexpr PieceType PROMOTION_PIECES[4] = {
    PieceType::Queen,   // Highest value - generated first for better alpha-beta cutoffs
    PieceType::Rook,
    PieceType::Bishop, 
    PieceType::Knight   // Lowest value - generated last
};

// Batch promotion move generation - generates all 4 promotion moves in tight loop
// This replaces the individual add_promotion_move calls that were causing overhead
inline void generate_promotion_batch(S_MOVELIST& list, int from, int to, PieceType captured = PieceType::None) {
    // Pre-calculate common values to avoid repeated computation
    const bool is_capture = (captured != PieceType::None);
    const int base_score = 2000000; // Base promotion score
    
    // Generate all 4 promotions in tight loop - compiler can optimize this better
    for (int i = 0; i < 4; ++i) {
        PieceType promoted = PROMOTION_PIECES[i];
        S_MOVE move = make_promotion(from, to, promoted, captured);
        
        // Optimized scoring calculation (avoiding function calls in hot path)
        int score = base_score;
        
        // Add promotion piece bonus (Queen=900, Rook=500, Bishop=350, Knight=300)
        switch (promoted) {
            case PieceType::Queen:  score += 90000; break;
            case PieceType::Rook:   score += 50000; break; 
            case PieceType::Bishop: score += 35000; break;
            case PieceType::Knight: score += 30000; break;
            default: break;
        }
        
        // Add capture bonus if applicable  
        if (is_capture) {
            switch (captured) {
                case PieceType::Queen:  score += 9000; break;
                case PieceType::Rook:   score += 5000; break;
                case PieceType::Bishop: score += 3500; break;
                case PieceType::Knight: score += 3000; break;
                case PieceType::Pawn:   score += 1000; break;
                default: break;
            }
        }
        
        // Add move directly to list (avoiding add_promotion_move overhead)
        list.moves[list.count] = move;
        list.moves[list.count].score = score;
        list.count++;
    }
}

// Specialized pawn move generation for promotion-heavy positions
// Uses early promotion square detection to avoid repeated rank calculations
inline void generate_pawn_moves_optimized(const Position& pos, S_MOVELIST& list, Color us) {
    const int piece_count = pos.pCount[int(us)][int(PieceType::Pawn)];
    
    // Early exit for positions with no pawns
    if (piece_count == 0) return;
    
    // Pre-calculate direction and ranks once
    const int direction = (us == Color::White) ? NORTH : SOUTH;
    const Rank start_rank = (us == Color::White) ? Rank::R2 : Rank::R7;
    // const Rank promo_rank = (us == Color::White) ? Rank::R8 : Rank::R1; // Not used - using promo_min/max instead
    
    // Pre-calculate promotion rank bounds for fast comparison
    const int promo_min = (us == Color::White) ? 91 : 21;
    const int promo_max = (us == Color::White) ? 98 : 28;
    
    for (int i = 0; i < piece_count; ++i) {
        const int from = pos.pList[int(us)][int(PieceType::Pawn)][i];
        if (from == -1) continue;
        
        // Single push
        const int to = from + direction;
        if (IS_PLAYABLE(to) && pos.at(to) == Piece::None) {
            // Fast promotion check using pre-calculated bounds
            if (to >= promo_min && to <= promo_max) {
                // Use optimized batch generation
                generate_promotion_batch(list, from, to);
            } else {
                list.add_quiet_move(make_move(from, to));
                
                // Double push from starting position
                if (rank_of(from) == start_rank) {
                    const int double_to = to + direction;
                    if (IS_PLAYABLE(double_to) && pos.at(double_to) == Piece::None) {
                        list.add_quiet_move(make_pawn_start(from, double_to));
                    }
                }
            }
        }
        
        // Captures (diagonals) - unrolled for better performance
        const int capture_left = from + direction + WEST;
        const int capture_right = from + direction + EAST;
        
        // Left capture
        if (IS_PLAYABLE(capture_left)) {
            const Piece target = pos.at(capture_left);
            if (target != Piece::None && color_of(target) == !us) {
                // Fast promotion check
                if (capture_left >= promo_min && capture_left <= promo_max) {
                    generate_promotion_batch(list, from, capture_left, type_of(target));
                } else {
                    list.add_capture_move(make_capture(from, capture_left, type_of(target)), pos);
                }
            } else if (capture_left == pos.ep_square) {
                list.add_en_passant_move(make_en_passant(from, capture_left));
            }
        }
        
        // Right capture
        if (IS_PLAYABLE(capture_right)) {
            const Piece target = pos.at(capture_right);
            if (target != Piece::None && color_of(target) == !us) {
                // Fast promotion check
                if (capture_right >= promo_min && capture_right <= promo_max) {
                    generate_promotion_batch(list, from, capture_right, type_of(target));
                } else {
                    list.add_capture_move(make_capture(from, capture_right, type_of(target)), pos);
                }
            } else if (capture_right == pos.ep_square) {
                list.add_en_passant_move(make_en_passant(from, capture_right));
            }
        }
    }
}

} // namespace PawnOptimizations
