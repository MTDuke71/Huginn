#pragma once
#include <cstdint>
#include <random>

#include "position.hpp"
#include "chess_types.hpp"
#include <cstdint>

using U64 = std::uint64_t;
constexpr int WKCA = 1;
constexpr int WQCA = 2;
constexpr int BKCA = 4;
constexpr int BQCA = 8;
constexpr int PIECE_NB = 12;

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

    inline U64 compute(const Position& b) {
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
                const int s64  = MAILBOX_MAPS.to64[s120];
                key ^= Piece[pc][s64];
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
} // namespace Zobrist
