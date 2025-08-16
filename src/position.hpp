#pragma once
#include <array>
#include <cstdint>
#include "board120.hpp"
#include "chess_types.hpp"
#include "move.hpp"

// Maximum search depth / game length
#define MAXPLY 2048

struct State {
    // Minimal saved state for unmake
    int ep_square;            // previous ep square
    uint8_t castling_rights;  // previous castling rights
    uint16_t halfmove_clock;  // previous halfmove clock
    Piece captured;           // captured piece (if any)
};

// Enhanced undo structure for comprehensive move history
struct S_UNDO {
    int move;                 // encoded move (from/to/promo packed)
    uint8_t castling_rights;  // previous castling permissions (castlePerm)
    int ep_square;            // previous en passant square (enPas)
    uint16_t halfmove_clock;  // previous fifty move counter (fiftyMove)
    uint64_t zobrist_key;     // previous position key (posKey)
    Piece captured;           // captured piece (if any)
    
    // Helper to encode/decode move
    static int encode_move(int from, int to, PieceType promo = PieceType::None) {
        return (from & 0x7F) | ((to & 0x7F) << 7) | ((int(promo) & 0x7) << 14);
    }
    
    static void decode_move(int encoded, int& from, int& to, PieceType& promo) {
        from = encoded & 0x7F;
        to = (encoded >> 7) & 0x7F;
        promo = PieceType((encoded >> 14) & 0x7);
    }
};

struct Position {
    std::array<Piece, 120> board{};  // Piece::None for empty/offboard
    Color side_to_move{Color::White};
    int ep_square{-1};               // mailbox-120 index or -1
    uint8_t castling_rights{0};      // bitmask: WKS=1, WQS=2, BKS=4, BQS=8
    uint16_t halfmove_clock{0};
    uint16_t fullmove_number{1};
    std::array<int, 2> king_sq{ -1, -1 }; // [White, Black] king locations (120)
    std::array<uint64_t, 2> pawns_bb{ 0, 0 }; // [White, Black] pawn bitboards (64)
    std::array<int, 7> piece_counts{}; // count by PieceType (None, Pawn, ..., King)
    uint64_t zobrist_key{0};
    
    // Move history for undo functionality - fixed array for performance
    std::array<S_UNDO, MAXPLY> move_history{};
    int ply{0};                      // current search/game ply

    void clear() {
        for (auto &p: board) p = Piece::None;
        side_to_move = Color::White;
        ep_square = -1;
        castling_rights = 0;
        halfmove_clock = 0;
        fullmove_number = 1;
        king_sq = { -1, -1 };
        pawns_bb = { 0, 0 };
        piece_counts.fill(0);
        zobrist_key = 0;
        // Clear move history by resetting ply to 0
        ply = 0;
    }

    // Put standard start position on 12x10
    void set_startpos() {
        clear();
        // Offboard frame already Piece::None; we only fill playable
        // White pieces
        board[sq(File::A, Rank::R1)] = Piece::WhiteRook;
        board[sq(File::B, Rank::R1)] = Piece::WhiteKnight;
        board[sq(File::C, Rank::R1)] = Piece::WhiteBishop;
        board[sq(File::D, Rank::R1)] = Piece::WhiteQueen;
        board[sq(File::E, Rank::R1)] = Piece::WhiteKing;
        board[sq(File::F, Rank::R1)] = Piece::WhiteBishop;
        board[sq(File::G, Rank::R1)] = Piece::WhiteKnight;
        board[sq(File::H, Rank::R1)] = Piece::WhiteRook;
        for (int f = 0; f < 8; ++f)
            board[sq(static_cast<File>(f), Rank::R2)] = Piece::WhitePawn;

        // Black pieces
        board[sq(File::A, Rank::R8)] = Piece::BlackRook;
        board[sq(File::B, Rank::R8)] = Piece::BlackKnight;
        board[sq(File::C, Rank::R8)] = Piece::BlackBishop;
        board[sq(File::D, Rank::R8)] = Piece::BlackQueen;
        board[sq(File::E, Rank::R8)] = Piece::BlackKing;
        board[sq(File::F, Rank::R8)] = Piece::BlackBishop;
        board[sq(File::G, Rank::R8)] = Piece::BlackKnight;
        board[sq(File::H, Rank::R8)] = Piece::BlackRook;
        for (int f = 0; f < 8; ++f)
            board[sq(static_cast<File>(f), Rank::R7)] = Piece::BlackPawn;

        side_to_move = Color::White;
        castling_rights = 0b1111; // KQkq
        ep_square = -1;
        halfmove_clock = 0;
        fullmove_number = 1;
        rebuild_counts();
    }

    // Update piece counts, king squares, pawn bitboards
    void rebuild_counts() {
        king_sq = { -1, -1 };
        pawns_bb = { 0, 0 };
        piece_counts.fill(0);
        for (int s = 0; s < 120; ++s) {
            Piece p = board[s];
            if (is_none(p)) continue;
            PieceType pt = type_of(p);
            ++piece_counts[size_t(pt)];
            Color c = color_of(p);
            if (pt == PieceType::Pawn && c != Color::None) {
                int s64 = MAILBOX_MAPS.to64[s];
                if (s64 >= 0) pawns_bb[size_t(c)] |= (1ULL << s64);
            }
            if (pt == PieceType::King && c != Color::None) {
                king_sq[size_t(c)] = s;
            }
        }
    }

    // Access
    inline Piece at(int s) const { return is_playable(s) ? board[size_t(s)] : Piece::None; }
    inline void set(int s, Piece p) { if (is_playable(s)) board[size_t(s)] = p; }
    
    // Enhanced move making with full undo support
    void make_move_with_undo(const Move& m) {
        // Check for ply overflow
        if (ply >= MAXPLY) {
            // Handle overflow - could throw exception or return false
            return; // For now, just return without making the move
        }
        
        S_UNDO& undo = move_history[ply]; // Direct array access
        undo.move = S_UNDO::encode_move(m.from, m.to, m.promo);
        undo.castling_rights = castling_rights;
        undo.ep_square = ep_square;
        undo.halfmove_clock = halfmove_clock;
        undo.zobrist_key = zobrist_key;
        undo.captured = at(m.to);
        
        // Increment ply after saving undo info
        ++ply;
        
        // Make the move
        Piece moving = at(m.from);
        if (type_of(moving) == PieceType::Pawn || !is_none(undo.captured)) {
            halfmove_clock = 0;
        } else {
            ++halfmove_clock;
        }
        
        set(m.to, moving);
        set(m.from, Piece::None);
        
        // Handle promotion
        if (m.promo != PieceType::None) {
            set(m.to, make_piece(color_of(moving), m.promo));
        }
        
        ep_square = -1; // Reset, update with double pawn push logic later
        side_to_move = !side_to_move;
        if (side_to_move == Color::White) ++fullmove_number;
        
        // Update derived state
        rebuild_counts();
        // Update zobrist_key here when Zobrist integration is complete
    }
    
    // Undo the last move
    bool undo_move() {
        if (ply == 0) return false; // No moves to undo
        
        // Decrement ply first to get the correct index
        --ply;
        S_UNDO& undo = move_history[ply]; // Direct array access
        
        // Decode the move
        int from, to;
        PieceType promo;
        S_UNDO::decode_move(undo.move, from, to, promo);
        
        // Undo side to move first
        side_to_move = !side_to_move;
        if (side_to_move == Color::Black) --fullmove_number;
        
        // Get the piece that moved
        Piece moved = at(to);
        
        // If it was a promotion, restore to pawn
        if (promo != PieceType::None) {
            moved = make_piece(color_of(moved), PieceType::Pawn);
        }
        
        // Move piece back
        set(from, moved);
        set(to, undo.captured); // Restore captured piece (or Piece::None)
        
        // Restore position state
        castling_rights = undo.castling_rights;
        ep_square = undo.ep_square;
        halfmove_clock = undo.halfmove_clock;
        zobrist_key = undo.zobrist_key;
        
        // Update derived state
        rebuild_counts();
        
        return true;
    }
};

// Minimal make/unmake stubs (quiet moves + captures only for now)
// Flesh these out as you implement move legality, en-passant, castling, promotions.
inline void make_move(Position& pos, const Move& m, State& st) {
    st.ep_square = pos.ep_square;
    st.castling_rights = pos.castling_rights;
    st.halfmove_clock = pos.halfmove_clock;
    st.captured = pos.at(m.to);

    Piece moving = pos.at(m.from);
    // update halfmove clock
    if (type_of(moving) == PieceType::Pawn || !is_none(st.captured)) pos.halfmove_clock = 0;
    else ++pos.halfmove_clock;

    pos.set(m.to, moving);
    pos.set(m.from, Piece::None);
    // promotions (simple)
    if (m.promo != PieceType::None) {
        pos.set(m.to, make_piece(color_of(moving), m.promo));
    }
    pos.ep_square = -1; // set by double push logic once you add it
    pos.side_to_move = !pos.side_to_move;
    if (pos.side_to_move == Color::White) ++pos.fullmove_number; // black just moved
}

inline void unmake_move(Position& pos, const Move& m, const State& st) {
    pos.side_to_move = !pos.side_to_move;
    if (pos.side_to_move == Color::Black) --pos.fullmove_number; // undo the increment
    Piece moved = pos.at(m.to);
    // If promo, restore pawn
    if (m.promo != PieceType::None) {
        moved = make_piece(color_of(moved), PieceType::Pawn);
    }
    pos.set(m.from, moved);
    pos.set(m.to, st.captured);
    pos.ep_square = st.ep_square;
    pos.castling_rights = st.castling_rights;
    pos.halfmove_clock = st.halfmove_clock;
}