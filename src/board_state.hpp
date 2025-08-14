#pragma once
#include <array>
#include <cstdint>
#include "board120.hpp"   // sq(), is_playable(), file_of(), MAILBOX_MAPS

using U64 = std::uint64_t;

// Sides and piece codes (classic tutorial-style)
enum Side : int { WHITE = 0, BLACK = 1, BOTH = 2 };
enum PieceCode : int {
    EMPTY = 0,
    wP, wN, wB, wR, wQ, wK,
    bP, bN, bB, bR, bQ, bK,
    PIECE_NB
};

inline constexpr int BRD_SQ_NUM = 120;
inline constexpr int NO_SQ = -1;
inline constexpr int OFFBOARD = -2;

// Castling rights mask (KQkq)
inline constexpr int WKCA = 1;
inline constexpr int WQCA = 2;
inline constexpr int BKCA = 4;
inline constexpr int BQCA = 8;

struct S_BOARD {
    std::array<int, BRD_SQ_NUM> pieces{};    // OFFBOARD on frame; EMPTY on playable empty; else PieceCode
    std::array<U64, 3>          pawns{};     // [WHITE], [BLACK], [BOTH] (64-bit A1=0..H8=63)
    std::array<int, 2>          KingSq{ NO_SQ, NO_SQ }; // in 120 indexing
    int side{WHITE};             // WHITE or BLACK
    int enPas{NO_SQ};            // en passant target (120) or NO_SQ
    int fiftyMove{0};
    int ply{0};
    int hisPly{0};
    U64 posKey{0};               // Zobrist
    std::array<int, PIECE_NB>    pceNum{};   // count by piece code
    std::array<int, 2>           bigPce{};   // non-pawns per side
    std::array<int, 2>           majPce{};   // rooks + queens per side
    std::array<int, 2>           minPce{};   // knights + bishops per side
    int castlePerm{0};           // bitmask WKCA|WQCA|BKCA|BQCA

    // Reset everything: frame=OFFBOARD; playable=EMPTY, counters zeroed
    void clear_board() {
        pieces.fill(OFFBOARD);
        for (int r = 0; r < 8; ++r)
            for (int f = 0; f < 8; ++f)
                pieces[ sq(static_cast<File>(f), static_cast<Rank>(r)) ] = EMPTY;

        pawns = {0,0,0};
        KingSq = { NO_SQ, NO_SQ };
        side = WHITE;
        enPas = NO_SQ;
        fiftyMove = ply = hisPly = 0;
        posKey = 0;
        pceNum.fill(0);
        bigPce = {0,0};
        majPce = {0,0};
        minPce = {0,0};
        castlePerm = 0;
    }
};

// --- Bitboard helpers on 64 squares (A1=0..H8=63)
inline void bb_set(U64& bb, int s64)        { bb |=  (1ULL << s64); }
inline void bb_clear(U64& bb, int s64)      { bb &= ~(1ULL << s64); }
inline bool bb_test(const U64& bb, int s64) { return (bb >> s64) & 1ULL; }

// Rebuild counters and pawn bitboards from pieces[120]
inline void rebuild_counts(S_BOARD& b) {
    b.pawns = {0,0,0};
    b.pceNum.fill(0);
    b.bigPce = {0,0};
    b.majPce = {0,0};
    b.minPce = {0,0};
    b.KingSq = {NO_SQ, NO_SQ};

    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            const int s120 = sq(static_cast<File>(f), static_cast<Rank>(r));
            const int pc   = b.pieces[ s120 ];
            if (pc == EMPTY) continue;

            const int s64 = MAILBOX_MAPS.to64[ s120 ];
            if (s64 < 0) continue; // should not happen

            ++b.pceNum[pc];

            switch (pc) {
                case wP: bb_set(b.pawns[WHITE], s64); break;
                case bP: bb_set(b.pawns[BLACK], s64); break;

                case wN: ++b.minPce[WHITE]; ++b.bigPce[WHITE]; break;
                case wB: ++b.minPce[WHITE]; ++b.bigPce[WHITE]; break;
                case wR: ++b.majPce[WHITE]; ++b.bigPce[WHITE]; break;
                case wQ: ++b.majPce[WHITE]; ++b.bigPce[WHITE]; break;
                case wK:  b.KingSq[WHITE] = s120;            ++b.bigPce[WHITE]; break;

                case bN: ++b.minPce[BLACK]; ++b.bigPce[BLACK]; break;
                case bB: ++b.minPce[BLACK]; ++b.bigPce[BLACK]; break;
                case bR: ++b.majPce[BLACK]; ++b.bigPce[BLACK]; break;
                case bQ: ++b.majPce[BLACK]; ++b.bigPce[BLACK]; break;
                case bK:  b.KingSq[BLACK] = s120;            ++b.bigPce[BLACK]; break;
            }
        }
    }
    b.pawns[BOTH] = b.pawns[WHITE] | b.pawns[BLACK];
}

// Place the standard start position
inline void set_startpos(S_BOARD& b) {
    b.clear_board();

    // White back rank
    b.pieces[ sq(File::A, Rank::R1) ] = wR;
    b.pieces[ sq(File::B, Rank::R1) ] = wN;
    b.pieces[ sq(File::C, Rank::R1) ] = wB;
    b.pieces[ sq(File::D, Rank::R1) ] = wQ;
    b.pieces[ sq(File::E, Rank::R1) ] = wK;
    b.pieces[ sq(File::F, Rank::R1) ] = wB;
    b.pieces[ sq(File::G, Rank::R1) ] = wN;
    b.pieces[ sq(File::H, Rank::R1) ] = wR;

    // White pawns
    for (int f = 0; f < 8; ++f)
        b.pieces[ sq(static_cast<File>(f), Rank::R2) ] = wP;

    // Black back rank
    b.pieces[ sq(File::A, Rank::R8) ] = bR;
    b.pieces[ sq(File::B, Rank::R8) ] = bN;
    b.pieces[ sq(File::C, Rank::R8) ] = bB;
    b.pieces[ sq(File::D, Rank::R8) ] = bQ;
    b.pieces[ sq(File::E, Rank::R8) ] = bK;
    b.pieces[ sq(File::F, Rank::R8) ] = bB;
    b.pieces[ sq(File::G, Rank::R8) ] = bN;
    b.pieces[ sq(File::H, Rank::R8) ] = bR;

    // Black pawns
    for (int f = 0; f < 8; ++f)
        b.pieces[ sq(static_cast<File>(f), Rank::R7) ] = bP;

    b.KingSq[WHITE] = sq(File::E, Rank::R1);
    b.KingSq[BLACK] = sq(File::E, Rank::R8);

    b.side = WHITE;
    b.enPas = NO_SQ;
    b.fiftyMove = 0;
    b.ply = b.hisPly = 0;
    b.castlePerm = WKCA | WQCA | BKCA | BQCA; // KQkq

    rebuild_counts(b);  // fills counters and pawns bitboards
}
