#pragma once
#include <cstdint>
#include "chess_types.hpp"

using U64 = std::uint64_t;
constexpr int PIECE_NB = 12;

// Forward declaration to avoid circular dependency
class Position;

namespace Zobrist {
    inline U64 Piece[PIECE_NB][120]; // piece on 120-square (direct indexing, no conversion needed)
    inline U64 Side;                 // side to move
    inline U64 Castle[16];           // castling rights mask (0..15)
    inline U64 EpFile[8];            // en-passant file a..h
    inline bool Initialized = false;

    inline void init_zobrist(std::uint64_t seed = 0x9E3779B97F4A7C15ULL) {
        if (Initialized) return;
        
        // SplitMix64 - faster and higher quality than Mersenne Twister for Zobrist
        class SplitMix64 {
            uint64_t state;
        public:
            explicit SplitMix64(uint64_t seed) : state(seed) {}
            uint64_t operator()() {
                uint64_t z = (state += 0x9e3779b97f4a7c15ULL);
                z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
                z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
                return z ^ (z >> 31);
            }
        } rng(seed);

        auto r64 = [&](){ return rng(); };

        for (int p = 0; p < PIECE_NB; ++p)
            for (int s = 0; s < 120; ++s)  // Initialize all 120 squares (includes off-board)
                Piece[p][s] = r64();

        Side = r64();
        for (int i = 0; i < 16; ++i) Castle[i] = r64();
        for (int f = 0; f < 8;  ++f) EpFile[f]  = r64();

        Initialized = true;
    }

    U64 compute(const Position& b);
} // namespace Zobrist
