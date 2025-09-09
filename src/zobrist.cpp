/**
 * @brief Computes the Zobrist hash key for a given chess position.
 *
 * This function performs a full board scan to calculate the Zobrist hash from scratch.
 * It iterates over all squares of the board, XOR-ing the corresponding Zobrist piece 
 * keys for each occupied square. It also incorporates the side to move, castling rights, 
 * and en passant file into the final hash key.
 *
 * @param b The Position object representing the current board state.
 * @return U64 The computed Zobrist hash key for the position.
 *
 * WHEN THIS FUNCTION IS CALLED:
 * - Position::set_from_fen() - Most common usage when loading positions from FEN
 * - Position::update_zobrist_key() - Called after position setup or mirroring
 * - Unit tests - For validating hash correctness and incremental updates
 * - Debug validation - Ensuring incremental updates match full computation
 * 
 * PERFORMANCE NOTE: 
 * This is an O(64) operation that scans the entire board. During actual gameplay,
 * the engine uses incremental Zobrist updates (O(1)) for better performance.
 *
 * The hash is constructed as follows:
 * - For each occupied square, XOR the precomputed Zobrist key for the piece and square.
 * - If the side to move is black, XOR the Zobrist key for side.
 * - XOR the Zobrist key for the current castling rights.
 * - If there is a valid en passant square, XOR the Zobrist key for the en passant file.
 */
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
