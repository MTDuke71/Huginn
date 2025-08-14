// src/main.cpp
#include <array>
#include <iostream>
#include <cassert>

#include "chess_types.hpp"  // Piece, Color, PieceType, helpers
#include "board120.hpp"     // File, Rank, sq(), is_playable, etc.
#include "squares120.hpp" // MailboxMaps, ALL64, ALL120, etc.
#include "board_state.hpp"
#include "zobrist.hpp"

int main() {
    // 120-cell mailbox board storing Pieces
    std::array<Piece, 120> board{};

    // Initialize to empty
    for (int i = 0; i < 120; ++i) board[i] = Piece::None;

    // Put some pieces down
    const int a1 = sq(File::A, Rank::R1);
    const int e1 = sq(File::E, Rank::R1);
    board[a1] = Piece::WhiteRook;
    board[e1] = Piece::WhiteKing;

    // Move example: e2 → e4 via +10 twice on mailbox-120
    const int e2 = sq(File::E, Rank::R2);
    const int e3 = e2 + 10;  // NORTH
    const int e4 = e3 + 10;  // NORTH
    assert(is_playable(e4));

    // Algebraic conversion
    char buf[3];
    to_algebraic(e4, buf); // "e4"
    std::cout << "Square e4 index: " << e4 << ", algebraic: " << buf << "\n";

    // Print a FEN char for the king on e1
    std::cout << "Piece at e1: " << to_char(board[e1]) << "\n"; // 'K'

    // Iterate all playable mailbox-120 squares
    for (int s : Playable120{}) {
        assert(is_playable(s));
    }

    // Translate 64→120 and back
    int s64  = 36;                    // e5 in 0..63 (A1=0)
    int s120 = MAILBOX_MAPS.to120[s64];
    int back = MAILBOX_MAPS.to64[s120];
    assert(back == s64);

    // Knight moves from g1
    int g1 = sq(File::G, Rank::R1);
    for (int d : KNIGHT_DELTAS) {
        int to = g1 + d;
        if (is_playable(to)) {
            // valid target on board
        }
    }

    // Sliding ray (rook north)
    int from = sq(File::E, Rank::R2);
    for (int to = from + NORTH; is_playable(to); to += NORTH) {
        // stop if blocked; otherwise keep extending
    }

    Zobrist::init_zobrist();  // call once at startup

    S_BOARD B;
    set_startpos(B);
    B.posKey = Zobrist::compute(B);

    // If you re-count later:
    rebuild_counts(B);
    const U64 check = Zobrist::compute(B);
    // assert(B.posKey == check); // once you maintain posKey incrementally
    return 0;
}
