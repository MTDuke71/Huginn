/**
 * @file pawn_lookup_tables.cpp
 * @brief Implementation of the pawn attack bitboard table
 *
 * Builds PAWN_ATTACKS[color][sq64] in pure 64-square file/rank
 * arithmetic. The five legacy `[2][120]` mailbox move tables were
 * deleted in the mailbox-120 elimination pass (they had no callers
 * after the S_MOVE 120->64 migration); this file no longer depends on
 * board120.hpp.
 *
 * ## Memory Usage
 * - 2 colors * 64 squares * 8 bytes = 1 KB total
 *
 * @author MTDuke71
 * @version 2.0
 */

#include "pawn_lookup_tables.hpp"
#include <iostream>

namespace PawnLookupTables {

// Bitboard attack tables - static allocation for performance
uint64_t PAWN_ATTACKS[2][64];

void initialize_pawn_tables() {
    for (int color = 0; color < 2; ++color) {
        for (int sq64 = 0; sq64 < 64; ++sq64) {
            PAWN_ATTACKS[color][sq64] = 0ULL;
        }
    }

    // A pawn attacks the two squares diagonally ahead of it. In sq64
    // space (file = sq64 % 8, rank = sq64 / 8, rank 0 = rank 1):
    //   White advances +8 per rank, so it attacks sq64+7 / sq64+9.
    //   Black advances -8 per rank, so it attacks sq64-9 / sq64-7.
    // The file guards (file > A / file < H) stop horizontal wrap; the
    // [0,64) bounds stop vertical overflow off the back/front rank.
    for (int sq64 = 0; sq64 < 64; ++sq64) {
        int file = sq64 % 8;

        uint64_t white_attacks = 0ULL;
        if (file > 0 && sq64 + 7 < 64) white_attacks |= (1ULL << (sq64 + 7));
        if (file < 7 && sq64 + 9 < 64) white_attacks |= (1ULL << (sq64 + 9));
        PAWN_ATTACKS[int(Color::White)][sq64] = white_attacks;

        uint64_t black_attacks = 0ULL;
        if (file > 0 && sq64 - 9 >= 0) black_attacks |= (1ULL << (sq64 - 9));
        if (file < 7 && sq64 - 7 >= 0) black_attacks |= (1ULL << (sq64 - 7));
        PAWN_ATTACKS[int(Color::Black)][sq64] = black_attacks;
    }

    std::cout << "Pawn lookup tables initialized successfully." << std::endl;
}

} // namespace PawnLookupTables
