/**
 * @file ultra_knight_gen.hpp
 * @brief Ultra-optimized knight move generation for pure 64-bit coordinates
 * 
 * First complete move generation implementation to validate our ultra-engine
 * approach against the current engine. Uses pure 64-bit coordinates and
 * ultra-fast attack tables for maximum performance.
 * 
 * @author MTDuke71
 * @version Ultra 1.0
 */
#pragma once
#include "ultra_move.hpp"
#include "ultra_attacks.hpp"

namespace UltraEngine {

// ============================================================================
// SIMPLE POSITION REPRESENTATION FOR TESTING
// Minimal position class to test knight move generation
// ============================================================================

class SimplePosition {
private:
    uint64_t piece_boards_[2][6];  // [color][piece_type]
    uint64_t color_boards_[2];     // [color] 
    uint64_t all_occupied_;
    int to_move_;

public:
    SimplePosition() {
        clear();
    }
    
    void clear() {
        for (int c = 0; c < 2; ++c) {
            for (int p = 0; p < 6; ++p) {
                piece_boards_[c][p] = 0ULL;
            }
            color_boards_[c] = 0ULL;
        }
        all_occupied_ = 0ULL;
        to_move_ = 0; // White to move
    }
    
    // Set up starting position for knights only (for testing)
    void setup_knights_only() {
        clear();
        
        // White knights on b1, g1
        piece_boards_[0][PieceType::Knight] = (1ULL << 1) | (1ULL << 6);
        color_boards_[0] = piece_boards_[0][PieceType::Knight];
        
        // Black knights on b8, g8  
        piece_boards_[1][PieceType::Knight] = (1ULL << 57) | (1ULL << 62);
        color_boards_[1] = piece_boards_[1][PieceType::Knight];
        
        all_occupied_ = color_boards_[0] | color_boards_[1];
    }
    
    // Add a piece at a square
    void add_piece(int square, int color, int piece_type) {
        const uint64_t sq_mask = 1ULL << square;
        piece_boards_[color][piece_type] |= sq_mask;
        color_boards_[color] |= sq_mask;
        all_occupied_ |= sq_mask;
    }
    
    // Remove a piece from a square
    void remove_piece(int square, int color, int piece_type) {
        const uint64_t sq_mask = ~(1ULL << square);
        piece_boards_[color][piece_type] &= sq_mask;
        color_boards_[color] &= sq_mask;
        // Recalculate all_occupied_
        all_occupied_ = color_boards_[0] | color_boards_[1];
    }
    
    // Accessors
    [[nodiscard]] uint64_t pieces(int color, int piece_type) const { return piece_boards_[color][piece_type]; }
    [[nodiscard]] uint64_t color_pieces(int color) const { return color_boards_[color]; }
    [[nodiscard]] uint64_t all_pieces() const { return all_occupied_; }
    [[nodiscard]] int side_to_move() const { return to_move_; }
    [[nodiscard]] const uint64_t (*piece_bitboards() const)[6] { return piece_boards_; }
    
    void set_side_to_move(int color) { to_move_ = color; }
    
    // Get piece type at square (returns -1 if empty)
    [[nodiscard]] int piece_at(int square) const {
        const uint64_t sq_mask = 1ULL << square;
        if (!(all_occupied_ & sq_mask)) return -1;
        
        for (int color = 0; color < 2; ++color) {
            if (!(color_boards_[color] & sq_mask)) continue;
            for (int piece = 0; piece < 6; ++piece) {
                if (piece_boards_[color][piece] & sq_mask) return piece;
            }
        }
        return -1;
    }
    
    // Get color at square (returns -1 if empty)
    [[nodiscard]] int color_at(int square) const {
        const uint64_t sq_mask = 1ULL << square;
        if (color_boards_[0] & sq_mask) return 0;
        if (color_boards_[1] & sq_mask) return 1;
        return -1;
    }
};

// ============================================================================
// ULTRA-FAST KNIGHT MOVE GENERATION
// ============================================================================

class SimpleKnightGen {
public:
    // Generate all legal knight moves for given color
    static void generate_knight_moves(const SimplePosition& pos, UltraMoveList& moves, int color) {
        const uint64_t knights = pos.pieces(color, PieceType::Knight);
        const uint64_t own_pieces = pos.color_pieces(color);
        const uint64_t enemy_pieces = pos.color_pieces(1 - color);
        
        uint64_t knight_bb = knights;
        while (knight_bb != 0) {
            const int from_sq = pop_lsb(knight_bb);
            
            // Get all possible knight attacks from this square
            const uint64_t attacks = UltraAttacks::knight(from_sq) & ~own_pieces;
            
            // Generate captures
            uint64_t captures = attacks & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                const int captured_piece = pos.piece_at(to_sq);
                moves.add_capture(from_sq, to_sq, captured_piece + 1);
            }
            
            // Generate quiet moves
            uint64_t quiet_moves = attacks & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
    }
    
    // Generate only knight captures for given color
    static void generate_knight_captures(const SimplePosition& pos, UltraMoveList& moves, int color) {
        const uint64_t knights = pos.pieces(color, PieceType::Knight);
        const uint64_t enemy_pieces = pos.color_pieces(1 - color);
        
        uint64_t knight_bb = knights;
        while (knight_bb != 0) {
            const int from_sq = pop_lsb(knight_bb);
            
            // Get knight attacks that hit enemy pieces
            uint64_t captures = UltraAttacks::knight(from_sq) & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                const int captured_piece = pos.piece_at(to_sq);
                moves.add_capture(from_sq, to_sq, captured_piece + 1);
            }
        }
    }
    
    // Generate only quiet knight moves for given color
    static void generate_knight_quiet(const SimplePosition& pos, UltraMoveList& moves, int color) {
        const uint64_t knights = pos.pieces(color, PieceType::Knight);
        const uint64_t own_pieces = pos.color_pieces(color);
        const uint64_t enemy_pieces = pos.color_pieces(1 - color);
        
        uint64_t knight_bb = knights;
        while (knight_bb != 0) {
            const int from_sq = pop_lsb(knight_bb);
            
            // Get knight attacks that don't hit any pieces
            uint64_t quiet_moves = UltraAttacks::knight(from_sq) & ~own_pieces & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
    }
    
    // Count knight moves without generating them (ultra-fast for perft)
    static int count_knight_moves(const SimplePosition& pos, int color) {
        const uint64_t knights = pos.pieces(color, PieceType::Knight);
        const uint64_t own_pieces = pos.color_pieces(color);
        
        int count = 0;
        uint64_t knight_bb = knights;
        while (knight_bb != 0) {
            const int from_sq = pop_lsb(knight_bb);
            const uint64_t attacks = UltraAttacks::knight(from_sq) & ~own_pieces;
            count += popcount(attacks);
        }
        
        return count;
    }
};

// ============================================================================
// ULTRA-KNIGHT MOVE GENERATION FOR TEMPLATE INTEGRATION
// Template-based knight generator for use with UltraPosition
// ============================================================================

template<typename Position>
class UltraKnightGen {
public:
    // Generate all knight moves (captures + quiet)
    static int generate_all(const Position& pos, UltraMoveList& moves, int color) {
        const int initial_count = moves.size();
        generate_captures(pos, moves, color);
        generate_quiet(pos, moves, color);
        return moves.size() - initial_count;
    }
    
    // Generate only knight captures
    static int generate_captures(const Position& pos, UltraMoveList& moves, int color) {
        const int initial_count = moves.size();
        const uint64_t knights = pos.get_piece_board(color, Position::Knight);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        
        uint64_t knight_bb = knights;
        while (knight_bb != 0) {
            const int from_sq = pop_lsb(knight_bb);
            
            // Get knight attacks that hit enemy pieces
            uint64_t captures = UltraAttacks::knight(from_sq) & enemy_pieces;
            while (captures != 0) {
                const int to_sq = pop_lsb(captures);
                const int captured_piece = pos.get_piece_type_at(to_sq);
                moves.add_capture(from_sq, to_sq, captured_piece + 1);
            }
        }
        return moves.size() - initial_count;
    }
    
    // Generate only quiet knight moves
    static int generate_quiet(const Position& pos, UltraMoveList& moves, int color) {
        const int initial_count = moves.size();
        const uint64_t knights = pos.get_piece_board(color, Position::Knight);
        const uint64_t own_pieces = pos.get_color_board(color);
        const uint64_t enemy_pieces = pos.get_color_board(1 - color);
        
        uint64_t knight_bb = knights;
        while (knight_bb != 0) {
            const int from_sq = pop_lsb(knight_bb);
            
            // Get knight attacks that don't hit any pieces
            uint64_t quiet_moves = UltraAttacks::knight(from_sq) & ~own_pieces & ~enemy_pieces;
            while (quiet_moves != 0) {
                const int to_sq = pop_lsb(quiet_moves);
                moves.add_quiet(from_sq, to_sq);
            }
        }
        return moves.size() - initial_count;
    }
    
    // Count knight moves without generating them (ultra-fast for perft)
    static int count_moves(const Position& pos, int color) {
        const uint64_t knights = pos.get_piece_board(color, Position::Knight);
        const uint64_t own_pieces = pos.get_color_board(color);
        
        int count = 0;
        uint64_t knight_bb = knights;
        while (knight_bb != 0) {
            const int from_sq = pop_lsb(knight_bb);
            const uint64_t attacks = UltraAttacks::knight(from_sq) & ~own_pieces;
            count += popcount(attacks);
        }
        
        return count;
    }
};

} // namespace UltraEngine