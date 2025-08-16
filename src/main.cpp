// src/main.cpp
#include <array>
#include <iostream>
#include <cassert>

#include "chess_types.hpp"  // Piece, Color, PieceType, helpers
#include "board120.hpp"     // File, Rank, sq(), is_playable, etc.
#include "squares120.hpp" // MailboxMaps, ALL64, ALL120, etc.
#include "position.hpp"
#include "zobrist.hpp"

int main() {
    // Modern board representation
    Position pos;
    pos.set_startpos();

    // Example: print algebraic for e4
    const int e4 = sq(File::E, Rank::R4);
    char buf[3];
    to_algebraic(e4, buf);
    std::cout << "Square e4 index: " << e4 << ", algebraic: " << buf << "\n";

    // Print a FEN char for the king on e1
    std::cout << "Piece at e1: " << to_char(pos.at(sq(File::E, Rank::R1))) << "\n";

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
    pos.zobrist_key = Zobrist::compute(pos);

    // If you re-count later:
    pos.rebuild_counts();
    const U64 check = Zobrist::compute(pos);
    // assert(pos.zobrist_key == check); // once you maintain zobrist_key incrementally
    return 0;
}
