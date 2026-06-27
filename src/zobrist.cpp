/**
 * @file zobrist.cpp
 * @brief Full-scan Zobrist key computation (the tables live in zobrist.hpp).
 *
 * Holds the out-of-line Zobrist::compute(); everything else (the key tables and
 * init_zobrist) is header-inline.
 */
#include "zobrist.hpp"
#include "position.hpp"

namespace Zobrist {
    /**
     * @brief Compute the Zobrist key for @p b from scratch by scanning all 64
     *        squares (XOR the per-piece/square key for each occupied square,
     *        plus side-to-move, castling rights, and en-passant file).
     *
     * O(64) — used for setup (set_from_fen, update_zobrist_key after mirroring)
     * and as the reference that validates the engine's O(1) incremental updates.
     * @param b Position to hash.
     * @return The full Zobrist key.
     */
    U64 compute(const Position& b) {
        U64 key = 0;
        // Pieces on board (iterate sq64 directly)
        for (int s64 = 0; s64 < 64; ++s64) {
            const auto piece = b.at_sq64(s64);
            if (is_none(piece)) continue;
            PieceType pt = type_of(piece);
            Color c = color_of(piece);
            int pc = int(pt) + (c == Color::Black ? 6 : 0);
            key ^= Piece[pc][s64];
        }
        if (b.side_to_move == Color::Black) key ^= Side;
        key ^= Castle[b.castling_rights & 0xF];
        if (b.ep_square != -1) {
            const int ff = b.ep_square & 7; // sq64 file 0..7
            if (ff >= 0 && ff < 8)
                key ^= EpFile[ff];
        }
        return key;
    }
}
