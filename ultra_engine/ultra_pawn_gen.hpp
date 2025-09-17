/**
 * @file ultra_pawn_gen.hpp
 * @brief Ultra-optimized pawn move generation for pure 64-bit coordinates
 * 
 * Provides blazing-fast pawn move generation including special rules:
 * - Single and double pushes
 * - Captures (including en passant)
 * - Promotions (queen, rook, bishop, knight)
 * - Bulk operations for maximum performance
 * 
 * @author MTDuke71
 * @version Ultra 1.0
 */
#pragma once
#include "ultra_move.hpp"
#include "ultra_attacks.hpp"
#include <type_traits>

namespace UltraEngine {

// ============================================================================
// ULTRA-PAWN GENERATION
// Template-based design for maximum flexibility and performance
// ============================================================================

template<typename Position>
class UltraPawnGen {
public:
    // ========================================================================
    // CORE PAWN MOVE GENERATION
    // All functions operate on pure 64-bit coordinates (0-63)
    // ========================================================================
    
    // Generate all pawn moves for a color
    static int generate_all(const Position& pos, UltraMoveList& moves, int color) {
        int count = 0;
        count += generate_pushes(pos, moves, color);
        count += generate_captures(pos, moves, color);
        return count;
    }
    
    // Generate only quiet pawn moves (pushes)
    static int generate_quiet(const Position& pos, UltraMoveList& moves, int color) {
        return generate_pushes(pos, moves, color);
    }
    
    // Generate only pawn captures
    static int generate_captures(const Position& pos, UltraMoveList& moves, int color) {
        int count = 0;
        count += generate_normal_captures(pos, moves, color);
        count += generate_en_passant(pos, moves, color);
        return count;
    }
    
    // ========================================================================
    // BULK OPERATIONS - Ultra-fast for perft
    // ========================================================================
    
    // Generate all pawn moves into single list (bulk operation)
    static int generate_bulk(const Position& pos, UltraMoveList& moves, int color) {
        return generate_all(pos, moves, color);
    }
    
    // Count pawn moves without storing (ultra-fast for perft)
    static int count_moves(const Position& pos, int color) {
        int count = 0;
        count += count_pushes(pos, color);
        count += count_captures(pos, color);
        return count;
    }
    
    // ========================================================================
    // SPECIALIZED PAWN OPERATIONS
    // ========================================================================
    
    // Generate only pawn pushes (single and double)
    static int generate_pushes(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t pawns = pos.get_piece_board(color, Position::Pawn);
        const uint64_t all_occupied = pos.get_all_occupied();
        const uint64_t empty = ~all_occupied;
        
        int count = 0;
        
        if (color == Position::White) {
            // White pawn pushes
            const uint64_t single_pushes = (pawns << 8) & empty;
            const uint64_t double_pushes = (single_pushes << 8) & empty & 0x00000000FF000000ULL; // 4th rank
            
            count += generate_push_moves(single_pushes, moves, color, false);
            count += generate_push_moves(double_pushes, moves, color, true);
        } else {
            // Black pawn pushes
            const uint64_t single_pushes = (pawns >> 8) & empty;
            const uint64_t double_pushes = (single_pushes >> 8) & empty & 0x000000FF00000000ULL; // 5th rank
            
            count += generate_push_moves(single_pushes, moves, color, false);
            count += generate_push_moves(double_pushes, moves, color, true);
        }
        
        return count;
    }
    
    // Generate normal pawn captures (no en passant)
    static int generate_normal_captures(const Position& pos, UltraMoveList& moves, int color) {
        const uint64_t pawns = pos.get_piece_board(color, Position::Pawn);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        
        int count = 0;
        
        if (color == Position::White) {
            // White pawn captures
            const uint64_t left_captures = (pawns << 7) & enemy_pieces & 0x7F7F7F7F7F7F7F7FULL; // Not A-file (pawns on A-file can't capture left)
            const uint64_t right_captures = (pawns << 9) & enemy_pieces & 0xFEFEFEFEFEFEFEFEULL; // Not H-file (pawns on H-file can't capture right)
            
            count += generate_capture_moves(left_captures, moves, color, -7);
            count += generate_capture_moves(right_captures, moves, color, -9);
        } else {
            // Black pawn captures
            const uint64_t left_captures = (pawns >> 9) & enemy_pieces & 0x7F7F7F7F7F7F7F7FULL; // Not A-file (pawns on A-file can't capture left)
            const uint64_t right_captures = (pawns >> 7) & enemy_pieces & 0xFEFEFEFEFEFEFEFEULL; // Not H-file (pawns on H-file can't capture right)
            
            count += generate_capture_moves(left_captures, moves, color, 9);
            count += generate_capture_moves(right_captures, moves, color, 7);
        }
        
        return count;
    }
    
    // Generate en passant captures
    static int generate_en_passant(const Position& pos, UltraMoveList& moves, int color) {
        const int ep_square = pos.get_en_passant_square();
        if (ep_square == -1) return 0;
        
        const uint64_t pawns = pos.get_piece_board(color, Position::Pawn);
        const uint64_t ep_target = 1ULL << ep_square;
        
        int count = 0;
        
        if (color == Position::White) {
            // White en passant captures
            const uint64_t ep_attackers = pawns & UltraAttacks::pawn(ep_square, Position::Black);
            count += generate_ep_moves(ep_attackers, moves, ep_square, color);
        } else {
            // Black en passant captures
            const uint64_t ep_attackers = pawns & UltraAttacks::pawn(ep_square, Position::White);
            count += generate_ep_moves(ep_attackers, moves, ep_square, color);
        }
        
        return count;
    }

private:
    // ========================================================================
    // MOVE GENERATION HELPERS
    // ========================================================================
    
    // Generate push moves from bitboard
    static int generate_push_moves(uint64_t targets, UltraMoveList& moves, int color, bool is_double) {
        int count = 0;
        
        while (targets) {
            const int to_square = UltraEngine::pop_lsb(targets);
            const int from_square = color == Position::White ? 
                (is_double ? to_square - 16 : to_square - 8) :
                (is_double ? to_square + 16 : to_square + 8);
            
            // Check for promotion
            if ((color == Position::White && to_square >= 56) || 
                (color == Position::Black && to_square <= 7)) {
                // Generate all promotion moves
                moves.add_promotion(from_square, to_square, Position::Queen);
                moves.add_promotion(from_square, to_square, Position::Rook);
                moves.add_promotion(from_square, to_square, Position::Bishop);
                moves.add_promotion(from_square, to_square, Position::Knight);
                count += 4;
            } else {
                // Normal pawn push
                if (is_double) {
                    moves.add_pawn_double(from_square, to_square);
                } else {
                    moves.add_quiet(from_square, to_square);
                }
                count++;
            }
        }
        
        return count;
    }
    
    // Generate capture moves from bitboard
    static int generate_capture_moves(uint64_t targets, UltraMoveList& moves, int color, int offset) {
        int count = 0;
        
        while (targets) {
            const int to_square = UltraEngine::pop_lsb(targets);
            const int from_square = to_square + offset;
            
            // Check for promotion
            if ((color == Position::White && to_square >= 56) || 
                (color == Position::Black && to_square <= 7)) {
                // Generate all promotion captures
                moves.add_promotion(from_square, to_square, Position::Queen, 1);
                moves.add_promotion(from_square, to_square, Position::Rook, 1);
                moves.add_promotion(from_square, to_square, Position::Bishop, 1);
                moves.add_promotion(from_square, to_square, Position::Knight, 1);
                count += 4;
            } else {
                // Normal pawn capture
                moves.add_capture(from_square, to_square, 1);
                count++;
            }
        }
        
        return count;
    }
    
    // Generate en passant moves
    static int generate_ep_moves(uint64_t attackers, UltraMoveList& moves, int ep_square, int color) {
        int count = 0;
        
        while (attackers) {
            const int from_square = UltraEngine::pop_lsb(attackers);
            moves.add_en_passant(from_square, ep_square);
            count++;
        }
        
        return count;
    }
    
    // ========================================================================
    // COUNTING HELPERS (for perft optimization)
    // ========================================================================
    
    // Count pawn pushes without storing moves
    static int count_pushes(const Position& pos, int color) {
        const uint64_t pawns = pos.get_piece_board(color, Position::Pawn);
        const uint64_t all_occupied = pos.get_all_occupied();
        const uint64_t empty = ~all_occupied;
        
        int count = 0;
        
        if (color == Position::White) {
            const uint64_t single_pushes = (pawns << 8) & empty;
            const uint64_t double_pushes = (single_pushes << 8) & empty & 0x00000000FF000000ULL;
            
            // Count promotions (multiply by 4)
            const uint64_t promotions = single_pushes & 0xFF00000000000000ULL;
            count += UltraEngine::popcount(single_pushes - promotions); // Normal pushes
            count += UltraEngine::popcount(promotions) * 4; // Promotions
            count += UltraEngine::popcount(double_pushes); // Double pushes
        } else {
            const uint64_t single_pushes = (pawns >> 8) & empty;
            const uint64_t double_pushes = (single_pushes >> 8) & empty & 0x000000FF00000000ULL;
            
            // Count promotions (multiply by 4)
            const uint64_t promotions = single_pushes & 0x00000000000000FFULL;
            count += UltraEngine::popcount(single_pushes - promotions); // Normal pushes
            count += UltraEngine::popcount(promotions) * 4; // Promotions
            count += UltraEngine::popcount(double_pushes); // Double pushes
        }
        
        return count;
    }
    
    // Count pawn captures without storing moves
    static int count_captures(const Position& pos, int color) {
        const uint64_t pawns = pos.get_piece_board(color, Position::Pawn);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        
        int count = 0;
        
        if (color == Position::White) {
            const uint64_t left_captures = (pawns << 7) & enemy_pieces & 0xFEFEFEFEFEFEFEFEULL;
            const uint64_t right_captures = (pawns << 9) & enemy_pieces & 0x7F7F7F7F7F7F7F7FULL;
            const uint64_t all_captures = left_captures | right_captures;
            
            // Count promotions (multiply by 4)
            const uint64_t promotions = all_captures & 0xFF00000000000000ULL;
            count += UltraEngine::popcount(all_captures - promotions); // Normal captures
            count += UltraEngine::popcount(promotions) * 4; // Promotion captures
        } else {
            const uint64_t left_captures = (pawns >> 9) & enemy_pieces & 0xFEFEFEFEFEFEFEFEULL;
            const uint64_t right_captures = (pawns >> 7) & enemy_pieces & 0x7F7F7F7F7F7F7F7FULL;
            const uint64_t all_captures = left_captures | right_captures;
            
            // Count promotions (multiply by 4)
            const uint64_t promotions = all_captures & 0x00000000000000FFULL;
            count += UltraEngine::popcount(all_captures - promotions); // Normal captures
            count += UltraEngine::popcount(promotions) * 4; // Promotion captures
        }
        
        // Add en passant if available
        const int ep_square = pos.get_en_passant_square();
        if (ep_square != -1) {
            const uint64_t ep_attackers = pawns & UltraAttacks::pawn(ep_square, 1 - color);
            count += UltraEngine::popcount(ep_attackers);
        }
        
        return count;
    }
};

// ============================================================================
// CONVENIENCE FUNCTIONS
// Direct access without template instantiation
// ============================================================================

template<typename Position>
inline int generate_pawn_moves(const Position& pos, UltraMoveList& moves, int color) {
    return UltraPawnGen<Position>::generate_all(pos, moves, color);
}

template<typename Position>
inline int generate_pawn_captures(const Position& pos, UltraMoveList& moves, int color) {
    return UltraPawnGen<Position>::generate_captures(pos, moves, color);
}

template<typename Position>
inline int generate_pawn_quiet(const Position& pos, UltraMoveList& moves, int color) {
    return UltraPawnGen<Position>::generate_quiet(pos, moves, color);
}

template<typename Position>
inline int count_pawn_moves(const Position& pos, int color) {
    return UltraPawnGen<Position>::count_moves(pos, color);
}

} // namespace UltraEngine