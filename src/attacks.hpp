#pragma once
#include "bitboard.hpp"

namespace Attacks {

    extern Bitboard ROOK_ATTACKS[64][4096];
    extern Bitboard ROOK_MASKS[64];
    extern const Bitboard ROOK_MAGICS[64];

    extern Bitboard BISHOP_ATTACKS[64][512];
    extern Bitboard BISHOP_MASKS[64];
    extern const Bitboard BISHOP_MAGICS[64];

    void init();

    inline Bitboard get_rook_attacks(int sq, Bitboard occupancy) {
        occupancy &= ROOK_MASKS[sq];
        occupancy *= ROOK_MAGICS[sq];
        occupancy >>= 64 - popcount(ROOK_MASKS[sq]);
        return ROOK_ATTACKS[sq][occupancy];
    }

    inline Bitboard get_bishop_attacks(int sq, Bitboard occupancy) {
        occupancy &= BISHOP_MASKS[sq];
        occupancy *= BISHOP_MAGICS[sq];
        occupancy >>= 64 - popcount(BISHOP_MASKS[sq]);
        return BISHOP_ATTACKS[sq][occupancy];
    }
}