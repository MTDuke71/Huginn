#pragma once
#include <cstdint>
#include <random>
#include "board_state.hpp" // S_BOARD, PieceCode, Side, etc.

namespace Zobrist {
    inline U64 Piece[PIECE_NB][64]; // piece on 64-square
    inline U64 Side;                // side to move
    inline U64 Castle[16];          // castling rights mask (0..15)
    inline U64 EpFile[8];           // en-passant file a..h
    inline bool Initialized = false;

    inline void init_zobrist(std::uint64_t seed = 0x9E3779B97F4A7C15ULL) {
        if (Initialized) return;
        std::mt19937_64 rng(seed);

        auto r64 = [&](){ return static_cast<U64>(rng()); };

        for (int p = 0; p < PIECE_NB; ++p)
            for (int s = 0; s < 64; ++s)
                Piece[p][s] = r64();

        Side = r64();
        for (int i = 0; i < 16; ++i) Castle[i] = r64();
        for (int f = 0; f < 8;  ++f) EpFile[f]  = r64();

        Initialized = true;
    }

    inline U64 compute(const S_BOARD& b) {
        U64 key = 0;

        // Pieces on board (iterate playable squares)
        for (int r = 0; r < 8; ++r) {
            for (int f = 0; f < 8; ++f) {
                const int s120 = sq(static_cast<File>(f), static_cast<Rank>(r));
                const int pc   = b.pieces[s120];
                if (pc == EMPTY) continue;
                const int s64  = MAILBOX_MAPS.to64[s120];
                key ^= Piece[pc][s64];
            }
        }

        if (b.side == BLACK) key ^= Side;

        key ^= Castle[b.castlePerm & 0xF];

        if (b.enPas != NO_SQ) {
            const int ff = static_cast<int>(file_of(b.enPas)); // 0..7
            if (ff >= 0 && ff < 8)
                key ^= EpFile[ff];
        }
        return key;
    }
} // namespace Zobrist
