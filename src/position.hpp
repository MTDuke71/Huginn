#pragma once
#include <array>
#include <cstdint>
#include "board120.hpp"
#include "chess_types.hpp"
#include "move.hpp"

struct State {
    // Minimal saved state for unmake
    int ep_square;            // previous ep square
    uint8_t castling_rights;  // previous castling rights
    uint16_t halfmove_clock;  // previous halfmove clock
    Piece captured;           // captured piece (if any)
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