#pragma once

#include "position.hpp"
#include "move.hpp"
#include "chess_types.hpp"
#include "board120.hpp"
#include "attack_detection.hpp"  // For SqAttacked function
#include "msvc_optimizations.hpp"
#include <algorithm>
#include <vector>

// Enhanced move generation with performance optimizations
#define MAX_POSITION_MOVES 256

struct S_MOVELIST {
    S_MOVE moves[MAX_POSITION_MOVES];  // Fixed-size array for better cache performance
    int count;
    
    // Constructor
    S_MOVELIST() : count(0) {}
    
    // Clear the move list
    void clear() { count = 0; }
    
    // Add methods for different move types with optimized scoring
    FORCE_INLINE void add_quiet_move(const S_MOVE& move) {
        // MSVC optimization: Tell compiler array bounds are safe
        __assume(count < MAX_POSITION_MOVES);
        moves[count] = move;
        moves[count].score = 0;  // Quiet moves get base score
        count++;
    }
    
    FORCE_INLINE void add_capture_move(const S_MOVE& move, const Position& pos) {
        __assume(count < MAX_POSITION_MOVES);
        moves[count] = move;
        // MVV-LVA scoring: Most Valuable Victim - Least Valuable Attacker
        Piece victim_piece = make_piece(!pos.side_to_move, move.get_captured());
        Piece attacker_piece = pos.at(move.get_from());
        moves[count].score = 1000000 + (10 * value_of(victim_piece)) - value_of(attacker_piece);
        count++;
    }
    
    FORCE_INLINE void add_en_passant_move(const S_MOVE& move) {
        __assume(count < MAX_POSITION_MOVES);
        moves[count] = move;
        moves[count].score = 1000105;  // En passant gets high priority (captures pawn)
        count++;
    }
    
    FORCE_INLINE void add_promotion_move(const S_MOVE& move) {
        __assume(count < MAX_POSITION_MOVES);
        moves[count] = move;
        // Promotion scoring based on promoted piece value
        Piece promo_piece = make_piece(Color::White, move.get_promoted());  // Color doesn't matter for value
        int promo_bonus = value_of(promo_piece) * 100;
        int capture_bonus = 0;
        if (move.is_capture()) [[likely]] {
            Piece captured_piece = make_piece(Color::White, move.get_captured());  // Color doesn't matter for value
            capture_bonus = value_of(captured_piece) * 10;
        }
        moves[count].score = 2000000 + promo_bonus + capture_bonus;
        count++;
    }
    
    FORCE_INLINE void add_castle_move(const S_MOVE& move) {
        __assume(count < MAX_POSITION_MOVES);
        moves[count] = move;
        moves[count].score = 50000;  // Castling gets moderate priority
        count++;
    }
    
    // Sort moves by score (highest first)
    void sort_by_score() {
        std::sort(moves, moves + count, [](const S_MOVE& a, const S_MOVE& b) {
            return a.score > b.score;
        });
    }
    
    // Access operators
    S_MOVE& operator[](int index) { return moves[index]; }
    const S_MOVE& operator[](int index) const { return moves[index]; }
    
    // Size accessor
    int size() const { return count; }
    
    // Iterator support for range-based loops
    S_MOVE* begin() { return moves; }
    S_MOVE* end() { return moves + count; }
    const S_MOVE* begin() const { return moves; }
    const S_MOVE* end() const { return moves + count; }
};

// Enhanced move generation function with improved organization
void generate_all_moves(const Position& pos, S_MOVELIST& list);

// ====================================================================
// Individual Piece Move Generation Functions
// ====================================================================
// NOTE: These functions are NOT used in production move generation.
// They are retained for profiling and testing purposes only.
//
// Production move generation uses optimized versions:
// - PawnOptimizations::generate_pawn_moves_optimized()
// - KnightOptimizations::generate_knight_moves_template()  
// - SlidingPieceOptimizations::generate_all_sliding_moves_optimized()
// - KingOptimizations::generate_king_moves_optimized()
// ====================================================================
void generate_pawn_moves(const Position& pos, S_MOVELIST& list, Color us);
void generate_knight_moves(const Position& pos, S_MOVELIST& list, Color us);
void generate_bishop_moves(const Position& pos, S_MOVELIST& list, Color us);
void generate_rook_moves(const Position& pos, S_MOVELIST& list, Color us);
void generate_queen_moves(const Position& pos, S_MOVELIST& list, Color us);
void generate_king_moves(const Position& pos, S_MOVELIST& list, Color us);

// Helper function for sliding pieces
void generate_sliding_moves(const Position& pos, S_MOVELIST& list, Color us, PieceType piece_type, const int* directions, int num_directions);

// Enhanced legal move generation with better performance
// Main move generation function  
void generate_legal_moves_enhanced(Position& pos, S_MOVELIST& list);

// VICE Part 65: Generate only capture moves for quiescence search
void generate_all_caps(Position& pos, S_MOVELIST& list);

// Helper functions
inline bool in_check(const Position& pos) {
    Color current_color = pos.side_to_move;
    int king_sq = pos.king_sq[int(current_color)];
    if (king_sq < 0) return false; // No king (shouldn't happen in valid position)
    
    // Check if the king is attacked by the opponent
    Color opponent_color = (current_color == Color::White) ? Color::Black : Color::White;
    return SqAttacked(king_sq, pos, opponent_color);
}

// Check if a move is legal (doesn't leave own king in check)
inline bool is_legal_move(Position& pos, const S_MOVE& move) {
    // Special handling for castling
    if (move.is_castle()) {
        int from = move.get_from();
        int to = move.get_to();
        Color current_side = pos.side_to_move;
        Color opponent_side = (current_side == Color::White) ? Color::Black : Color::White;
        
        // King cannot be in check before castling
        if (SqAttacked(from, pos, opponent_side)) {
            return false;
        }
        
        // Check that king doesn't pass through attacked squares during castling
        int step = (to > from) ? 1 : -1;
        for (int sq = from + step; sq != to + step; sq += step) {
            if (SqAttacked(sq, pos, opponent_side)) {
                return false;
            }
        }
        return true;
    }
    
    // For all other moves, use the proper move/undo system
    Color current_side = pos.side_to_move;
    
    // Apply the move using VICE method
    if (pos.MakeMove(move) != 1) {
        return false;  // Move was illegal
    }
    
    // Get the king position after the move (for the side that just moved)
    int king_sq = pos.king_sq[int(current_side)];
    
    // Check if our king would be in check after the move
    // Note: after the move, it's the opponent's turn, so we check if opponent attacks our king
    bool legal = !SqAttacked(king_sq, pos, !current_side);
    
    // Undo the move to restore the original position
    pos.TakeMove();
    
    return legal;
}
