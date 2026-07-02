/**
 * @file zobrist.hpp
 * @brief Zobrist hashing utilities for chess positions.
 *
 * This header defines the Zobrist namespace, which provides functions and tables
 * for generating and managing Zobrist hashes used to uniquely identify chess positions.
 * 
 * Zobrist hashing is a technique for efficiently computing a unique hash for a chess
 * position, taking into account piece placement, side to move, castling rights, and
 * en-passant possibilities.
 *
 * Globals:
 * - Piece[PIECE_NB][64]: Zobrist keys for each piece type on each square (0..63).
 * - Side: Zobrist key for the side to move.
 * - Castle[16]: Zobrist keys for each possible castling rights combination (0..15).
 * - EpFile[8]: Zobrist keys for en-passant files (a..h).
 * - Initialized: Flag indicating whether the Zobrist tables have been initialized.
 *
 * Functions:
 * - void init_zobrist(std::uint64_t seed = 0x9E3779B97F4A7C15ULL):
 *     Initializes the Zobrist tables with pseudo-random values using SplitMix64.
 *     Should be called once before using Zobrist hashing.
 * - U64 compute(const Position& b):
 *     Computes the Zobrist hash for the given chess position.
 *
 * @note The SplitMix64 PRNG is used for fast and high-quality random number generation.
 * @note The Piece table uses 64 squares directly.
 * @note Forward declaration of Position is used to avoid circular dependencies.
 */
#pragma once
#include <cstdint>
#include "chess_types.hpp"

using U64 = std::uint64_t;

// BACKLOG #50: number of rows in the Piece key table. The piece-index scheme
// used by every keying site (position.hpp make/unmake primitives,
// Zobrist::compute, Position::is_consistent) is
//     row = int(PieceType) + (Black ? 6 : 0)      // Pawn=1 .. King=6
// i.e. White 1..6, Black 7..12, row 0 unused — 13 rows. The historical
// PIECE_NB=12 made the BLACK KING (row 12) index one row PAST the table:
// an out-of-bounds read of whatever globals the linker placed after it. On
// the MSVC release layout that region held an ASLR'd heap pointer at slot 29
// (= black king on f4), injecting a per-process value into the zobrist key —
// the fixed-depth node-count nondeterminism of BACKLOG #50 (Kiwipete d14
// wobbled ~1k in 4.7M run-to-run; startpos never walks the black king to f4
// and stayed byte-identical). Several other slots read constant ZERO, so
// positions differing only in those black-king squares hashed identically —
// real TT key collisions. Flag-off reproduces the t22 baseline arm (UB and
// all) for A/B comparison only: build with -DENABLE_ZOBRIST_BLACK_KING_ROW=0.
#ifndef ENABLE_ZOBRIST_BLACK_KING_ROW
#define ENABLE_ZOBRIST_BLACK_KING_ROW 1
#endif
#if ENABLE_ZOBRIST_BLACK_KING_ROW
constexpr int PIECE_NB = 13;  // rows 1..12 used (White 1-6, Black 7-12); 0 unused
#else
constexpr int PIECE_NB = 12;  // baseline-t22: black king (row 12) reads OOB — UB
#endif

// Forward declaration to avoid circular dependency - ensure consistency with position.hpp
class Position;

namespace Zobrist {
    inline U64 Piece[PIECE_NB][64];  ///< Key per [piece-type][sq64]; White rows 1-6, Black 7-12.
    inline U64 Side;                 ///< Key XOR'd in when Black is to move.
    inline U64 Castle[16];           ///< Key per castling-rights mask (0..15).
    inline U64 EpFile[8];            ///< Key per en-passant file (a..h).
    inline bool Initialized = false; ///< True once init_zobrist() has run.

    /// @brief Fill the key tables with SplitMix64 pseudo-random values. Idempotent
    ///        (no-op if already initialized). Call once at startup before hashing.
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
            for (int s = 0; s < 64; ++s)
                Piece[p][s] = r64();

        Side = r64();
        for (int i = 0; i < 16; ++i) Castle[i] = r64();
        for (int f = 0; f < 8;  ++f) EpFile[f]  = r64();

        Initialized = true;
    }

    /// @brief Compute the full Zobrist key for @p b from scratch (pieces + side
    ///        + castling + ep). The search maintains the key incrementally; this
    ///        is for setup/rebuild and as the incremental-update reference.
    U64 compute(const Position& b);
} // namespace Zobrist
