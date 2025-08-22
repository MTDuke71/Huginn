#ifndef MOVEGEN_ENHANCED_HPP
#define MOVEGEN_ENHANCED_HPP

#include "position.hpp"
#include "move.hpp"
#include "chess_types.hpp"
#include "board120.hpp"
#include <algorithm>

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
    void add_quiet_move(const S_MOVE& move) {
        moves[count] = move;
        moves[count].score = 0;  // Quiet moves get base score
        count++;
    }
    
    void add_capture_move(const S_MOVE& move, const Position& pos) {
        moves[count] = move;
        // MVV-LVA scoring: Most Valuable Victim - Least Valuable Attacker
        Piece victim_piece = make_piece(!pos.side_to_move, move.get_captured());
        Piece attacker_piece = pos.at(move.get_from());
        moves[count].score = 1000000 + (10 * value_of(victim_piece)) - value_of(attacker_piece);
        count++;
    }
    
    void add_en_passant_move(const S_MOVE& move) {
        moves[count] = move;
        moves[count].score = 1000105;  // En passant gets high priority (captures pawn)
        count++;
    }
    
    void add_promotion_move(const S_MOVE& move) {
        moves[count] = move;
        // Promotion scoring based on promoted piece value
        Piece promo_piece = make_piece(Color::White, move.get_promoted());  // Color doesn't matter for value
        int promo_bonus = value_of(promo_piece) * 100;
        int capture_bonus = 0;
        if (move.is_capture()) {
            Piece captured_piece = make_piece(Color::White, move.get_captured());  // Color doesn't matter for value
            capture_bonus = value_of(captured_piece) * 10;
        }
        moves[count].score = 2000000 + promo_bonus + capture_bonus;
        count++;
    }
    
    void add_castle_move(const S_MOVE& move) {
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

// Forward declarations
class Position;

// Enhanced move generation function with improved organization
void generate_all_moves(const Position& pos, S_MOVELIST& list);

// Specialized move generation functions for each piece type
void generate_pawn_moves(const Position& pos, S_MOVELIST& list, Color us);
void generate_knight_moves(const Position& pos, S_MOVELIST& list, Color us);
void generate_bishop_moves(const Position& pos, S_MOVELIST& list, Color us);
void generate_rook_moves(const Position& pos, S_MOVELIST& list, Color us);
void generate_queen_moves(const Position& pos, S_MOVELIST& list, Color us);
void generate_king_moves(const Position& pos, S_MOVELIST& list, Color us);

// Helper function for sliding pieces
void generate_sliding_moves(const Position& pos, S_MOVELIST& list, Color us, PieceType piece_type, const int* directions, int num_directions);

// Enhanced legal move generation with better performance
void generate_legal_moves_enhanced(const Position& pos, S_MOVELIST& list);

#endif // MOVEGEN_ENHANCED_HPP
