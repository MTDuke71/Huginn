/**
 * @file ultra_position.hpp
 * @brief Ultra-optimized chess position representation for pure 64-bit coordinates
 * 
 * Provides blazing-fast position management compatible with all ultra-engine components:
 * - Pure bitboard representation (64-bit coordinates)
 * - Zero coordinate conversions
 * - Complete move generation integration
 * - Ultra-fast make/unmake moves
 * - Perft-optimized bulk operations
 * 
 * @author MTDuke71
 * @version Ultra 1.0
 */
#pragma once
#include "ultra_move.hpp"
#include "ultra_attacks.hpp"
#include "ultra_knight_gen.hpp"
#include "ultra_sliding_gen.hpp"
#include "ultra_pawn_gen.hpp"
#include <array>
#include <string>
#include <cassert>

namespace UltraEngine {

// ============================================================================
// ULTRA-POSITION SYSTEM
// Complete chess position with pure 64-bit coordinate system
// ============================================================================

class UltraPosition {
public:
    // ========================================================================
    // PIECE AND COLOR CONSTANTS
    // ========================================================================
    static constexpr int Pawn = 0;
    static constexpr int Knight = 1;
    static constexpr int Bishop = 2;
    static constexpr int Rook = 3;
    static constexpr int Queen = 4;
    static constexpr int King = 5;
    
    static constexpr int White = 0;
    static constexpr int Black = 1;
    
    static constexpr int NUM_PIECE_TYPES = 6;
    static constexpr int NUM_COLORS = 2;

private:
    // ========================================================================
    // CORE BITBOARD REPRESENTATION
    // ========================================================================
    std::array<std::array<uint64_t, NUM_PIECE_TYPES>, NUM_COLORS> piece_boards_;
    std::array<uint64_t, NUM_COLORS> color_boards_;
    uint64_t all_occupied_;
    
    // ========================================================================
    // GAME STATE
    // ========================================================================
    int to_move_;
    int en_passant_square_;
    uint8_t castling_rights_; // bits: 0=wK, 1=wQ, 2=bK, 3=bQ
    int halfmove_clock_;
    int fullmove_number_;
    
    // ========================================================================
    // POSITION HISTORY (for efficient undo)
    // ========================================================================
    struct StateInfo {
        int en_passant_square;
        uint8_t castling_rights;
        int halfmove_clock;
        uint64_t captured_piece_bb;
        int captured_piece_type;
        uint64_t hash_key; // For repetition detection
    };
    
    std::array<StateInfo, 1024> state_stack_; // Deep enough for any game
    int state_index_;

public:
    // ========================================================================
    // CONSTRUCTORS AND SETUP
    // ========================================================================
    
    UltraPosition() { clear(); }
    
    explicit UltraPosition(const std::string& fen) {
        clear();
        set_fen(fen);
    }
    
    void clear() {
        // Clear all bitboards
        for (int color = 0; color < NUM_COLORS; color++) {
            color_boards_[color] = 0ULL;
            for (int piece = 0; piece < NUM_PIECE_TYPES; piece++) {
                piece_boards_[color][piece] = 0ULL;
            }
        }
        all_occupied_ = 0ULL;
        
        // Reset game state
        to_move_ = White;
        en_passant_square_ = -1;
        castling_rights_ = 0;
        halfmove_clock_ = 0;
        fullmove_number_ = 1;
        state_index_ = 0;
    }
    
    void setup_starting_position() {
        clear();
        set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }
    
    // ========================================================================
    // BITBOARD ACCESS - Ultra-fast inline methods
    // ========================================================================
    
    [[nodiscard]] uint64_t get_piece_board(int color, int piece_type) const noexcept {
        return piece_boards_[color][piece_type];
    }
    
    [[nodiscard]] uint64_t get_color_board(int color) const noexcept {
        return color_boards_[color];
    }
    
    [[nodiscard]] uint64_t get_all_occupied() const noexcept {
        return all_occupied_;
    }
    
    [[nodiscard]] int get_to_move() const noexcept {
        return to_move_;
    }
    
    [[nodiscard]] int get_en_passant_square() const noexcept {
        return en_passant_square_;
    }
    
    [[nodiscard]] uint8_t get_castling_rights() const noexcept {
        return castling_rights_;
    }
    
    [[nodiscard]] int get_halfmove_clock() const noexcept {
        return halfmove_clock_;
    }
    
    [[nodiscard]] int get_fullmove_number() const noexcept {
        return fullmove_number_;
    }
    
    // ========================================================================
    // PIECE PLACEMENT AND REMOVAL
    // ========================================================================
    
    void set_piece(int square, int color, int piece_type) {
        assert(square >= 0 && square < 64);
        assert(color >= 0 && color < NUM_COLORS);
        assert(piece_type >= 0 && piece_type < NUM_PIECE_TYPES);
        
        const uint64_t sq_bb = 1ULL << square;
        piece_boards_[color][piece_type] |= sq_bb;
        color_boards_[color] |= sq_bb;
        all_occupied_ |= sq_bb;
    }
    
    void remove_piece(int square) {
        assert(square >= 0 && square < 64);
        
        const uint64_t sq_mask = ~(1ULL << square);
        for (int color = 0; color < NUM_COLORS; color++) {
            color_boards_[color] &= sq_mask;
            for (int piece = 0; piece < NUM_PIECE_TYPES; piece++) {
                piece_boards_[color][piece] &= sq_mask;
            }
        }
        all_occupied_ &= sq_mask;
    }
    
    [[nodiscard]] int get_piece_at(int square) const {
        assert(square >= 0 && square < 64);
        
        const uint64_t sq_bb = 1ULL << square;
        if (!(all_occupied_ & sq_bb)) return -1; // Empty square
        
        for (int color = 0; color < NUM_COLORS; color++) {
            if (color_boards_[color] & sq_bb) {
                for (int piece = 0; piece < NUM_PIECE_TYPES; piece++) {
                    if (piece_boards_[color][piece] & sq_bb) {
                        return (color << 3) | piece; // Encode color and piece
                    }
                }
            }
        }
        return -1; // Should never reach here
    }
    
    [[nodiscard]] int get_piece_color_at(int square) const {
        const int piece_info = get_piece_at(square);
        return piece_info >= 0 ? (piece_info >> 3) : -1;
    }
    
    [[nodiscard]] int get_piece_type_at(int square) const {
        const int piece_info = get_piece_at(square);
        return piece_info >= 0 ? (piece_info & 7) : -1;
    }
    
    // ========================================================================
    // COMPLETE MOVE GENERATION - Integration of all components
    // ========================================================================
    
    int generate_all_moves(UltraMoveList& moves) const {
        int count = 0;
        const int color = to_move_;
        
        // Generate moves for all piece types
        count += UltraPawnGen<UltraPosition>::generate_all(*this, moves, color);
        count += UltraKnightGen<UltraPosition>::generate_all(*this, moves, color);
        count += UltraSlidingGen<UltraPosition>::generate_all(*this, moves, color);
        count += generate_king_moves(moves, color);
        count += generate_castling_moves(moves, color);
        
        return count;
    }
    
    int generate_captures(UltraMoveList& moves) const {
        int count = 0;
        const int color = to_move_;
        
        count += UltraPawnGen<UltraPosition>::generate_captures(*this, moves, color);
        count += UltraKnightGen<UltraPosition>::generate_captures(*this, moves, color);
        count += UltraSlidingGen<UltraPosition>::generate_captures(*this, moves, color);
        count += generate_king_captures(moves, color);
        
        return count;
    }
    
    int generate_quiet_moves(UltraMoveList& moves) const {
        int count = 0;
        const int color = to_move_;
        
        count += UltraPawnGen<UltraPosition>::generate_quiet(*this, moves, color);
        count += UltraKnightGen<UltraPosition>::generate_quiet(*this, moves, color);
        count += UltraSlidingGen<UltraPosition>::generate_quiet(*this, moves, color);
        count += generate_king_quiet(moves, color);
        count += generate_castling_moves(moves, color);
        
        return count;
    }
    
    // ========================================================================
    // ULTRA-FAST MOVE COUNTING (for perft optimization)
    // ========================================================================
    
    int count_all_moves() const {
        int count = 0;
        const int color = to_move_;
        
        count += UltraPawnGen<UltraPosition>::count_moves(*this, color);
        count += UltraKnightGen<UltraPosition>::count_moves(*this, color);
        count += UltraSlidingGen<UltraPosition>::count_moves(*this, color);
        count += count_king_moves(color);
        count += count_castling_moves(color);
        
        return count;
    }
    
    // ========================================================================
    // MAKE/UNMAKE MOVES - Ultra-fast with full state preservation
    // ========================================================================
    
    void make_move(const UltraMove& move) {
        // Save current state
        StateInfo& state = state_stack_[state_index_++];
        state.en_passant_square = en_passant_square_;
        state.castling_rights = castling_rights_;
        state.halfmove_clock = halfmove_clock_;
        state.captured_piece_bb = 0ULL;
        state.captured_piece_type = -1;
        
        const int from = move.from();
        const int to = move.to();
        const int moving_color = to_move_;
        const int moving_piece = get_piece_type_at(from);
        
        // Handle captures
        if (move.is_capture() || move.is_ep()) {
            if (move.is_ep()) {
                // En passant capture
                const int captured_square = (moving_color == White) ? to - 8 : to + 8;
                state.captured_piece_bb = 1ULL << captured_square;
                state.captured_piece_type = Pawn;
                remove_piece(captured_square);
            } else {
                // Normal capture
                state.captured_piece_bb = 1ULL << to;
                state.captured_piece_type = get_piece_type_at(to);
                remove_piece(to);
            }
            halfmove_clock_ = 0;
        } else if (moving_piece == Pawn) {
            halfmove_clock_ = 0;
        } else {
            halfmove_clock_++;
        }
        
        // Move the piece
        remove_piece(from);
        
        if (move.is_promotion()) {
            set_piece(to, moving_color, move.promoted());
        } else {
            set_piece(to, moving_color, moving_piece);
        }
        
        // Handle special moves
        if (move.is_castle()) {
            handle_castling(from, to);
        }
        
        // Update en passant
        en_passant_square_ = -1;
        if (move.is_pawn_start()) {
            en_passant_square_ = (moving_color == White) ? from + 8 : from - 8;
        }
        
        // Update castling rights
        update_castling_rights(from, to, moving_piece);
        
        // Switch sides
        to_move_ = 1 - to_move_;
        if (to_move_ == White) fullmove_number_++;
        
        // Rebuild derived bitboards
        rebuild_derived_bitboards();
    }
    
    void unmake_move(const UltraMove& move) {
        // Switch sides back
        to_move_ = 1 - to_move_;
        if (to_move_ == Black) fullmove_number_--;
        
        const StateInfo& state = state_stack_[--state_index_];
        const int from = move.from();
        const int to = move.to();
        const int moving_color = to_move_;
        
        // Restore piece position
        if (move.is_promotion()) {
            remove_piece(to);
            set_piece(from, moving_color, Pawn);
        } else {
            const int moving_piece = get_piece_type_at(to);
            remove_piece(to);
            set_piece(from, moving_color, moving_piece);
        }
        
        // Handle special moves
        if (move.is_castle()) {
            handle_uncastling(from, to);
        }
        
        // Restore captured piece
        if (state.captured_piece_bb) {
            const int captured_square = get_lsb(state.captured_piece_bb);
            set_piece(captured_square, 1 - moving_color, state.captured_piece_type);
        }
        
        // Restore game state
        en_passant_square_ = state.en_passant_square;
        castling_rights_ = state.castling_rights;
        halfmove_clock_ = state.halfmove_clock;
        
        // Rebuild derived bitboards
        rebuild_derived_bitboards();
    }
    
    // ========================================================================
    // FEN SUPPORT
    // ========================================================================
    
    bool set_fen(const std::string& fen);
    std::string get_fen() const;
    
    // ========================================================================
    // GAME STATE QUERIES
    // ========================================================================
    
    [[nodiscard]] bool is_in_check() const {
        const int king_square = get_lsb(piece_boards_[to_move_][King]);
        return UltraAttacks::is_square_attacked(king_square, 1 - to_move_, 
                                               (const uint64_t(*)[6])piece_boards_.data(), all_occupied_);
    }
    
    [[nodiscard]] bool is_legal_move(const UltraMove& move) const;
    [[nodiscard]] bool is_checkmate() const;
    [[nodiscard]] bool is_stalemate() const;
    [[nodiscard]] bool is_draw() const;

private:
    // ========================================================================
    // INTERNAL HELPERS
    // ========================================================================
    
    void rebuild_derived_bitboards() {
        color_boards_[White] = 0ULL;
        color_boards_[Black] = 0ULL;
        all_occupied_ = 0ULL;
        
        for (int color = 0; color < NUM_COLORS; color++) {
            for (int piece = 0; piece < NUM_PIECE_TYPES; piece++) {
                color_boards_[color] |= piece_boards_[color][piece];
            }
            all_occupied_ |= color_boards_[color];
        }
    }
    
    // King move generation helpers
    int generate_king_moves(UltraMoveList& moves, int color) const;
    int generate_king_captures(UltraMoveList& moves, int color) const;
    int generate_king_quiet(UltraMoveList& moves, int color) const;
    int count_king_moves(int color) const;
    
    // Castling helpers
    int generate_castling_moves(UltraMoveList& moves, int color) const;
    int count_castling_moves(int color) const;
    void handle_castling(int from, int to);
    void handle_uncastling(int from, int to);
    void update_castling_rights(int from, int to, int piece_type);
    
    // FEN parsing helpers
    int char_to_piece(char c) const;
    char piece_to_char(int color, int piece) const;
};

// ============================================================================
// INLINE IMPLEMENTATIONS - Critical path functions
// ============================================================================

inline int UltraPosition::generate_king_moves(UltraMoveList& moves, int color) const {
    const uint64_t king_bb = piece_boards_[color][King];
    if (!king_bb) return 0;
    
    const int king_square = get_lsb(king_bb);
    const uint64_t attacks = UltraAttacks::king(king_square);
    const uint64_t enemy_pieces = color_boards_[1 - color];
    const uint64_t own_pieces = color_boards_[color];
    
    int count = 0;
    uint64_t targets = attacks & ~own_pieces;
    
    while (targets) {
        const int to_square = pop_lsb(targets);
        if (enemy_pieces & (1ULL << to_square)) {
            moves.add_capture(king_square, to_square, 1);
        } else {
            moves.add_quiet(king_square, to_square);
        }
        count++;
    }
    
    return count;
}

inline int UltraPosition::count_king_moves(int color) const {
    const uint64_t king_bb = piece_boards_[color][King];
    if (!king_bb) return 0;
    
    const int king_square = get_lsb(king_bb);
    const uint64_t attacks = UltraAttacks::king(king_square);
    const uint64_t own_pieces = color_boards_[color];
    
    return popcount(attacks & ~own_pieces);
}

} // namespace UltraEngine