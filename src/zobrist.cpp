// zobrist.cpp - Implementation file for Zobrist compute function
#include "zobrist.hpp"
#include "position.hpp"

namespace Zobrist {
    U64 compute(const Position& b) {
        U64 key = 0;
        // Pieces on board (iterate playable squares)
        for (int r = 0; r < 8; ++r) {
            for (int f = 0; f < 8; ++f) {
                const int s120 = sq(static_cast<File>(f), static_cast<Rank>(r));
                const auto piece = b.at(s120);
                if (is_none(piece)) continue;
                PieceType pt = type_of(piece);
                Color c = color_of(piece);
                int pc = int(pt) + (c == Color::Black ? 6 : 0); // Map to legacy index if needed
                key ^= Piece[pc][s120];  // Direct sq120 lookup - no conversion needed!
            }
        }
        if (b.side_to_move == Color::Black) key ^= Side;
        key ^= Castle[b.castling_rights & 0xF];
        if (b.ep_square != -1) {
            const int ff = static_cast<int>(file_of(b.ep_square)); // 0..7
            if (ff >= 0 && ff < 8)
                key ^= EpFile[ff];
        }
        return key;
    }
}
