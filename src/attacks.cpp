#include "attacks.hpp"
#include "bitboard.hpp"

namespace Attacks {

    // Magic numbers for rook attack generation
    const Bitboard ROOK_MAGICS[64] = {
        0x8a80104000800020ULL, 0x140002000100040ULL, 0x2800010000402000ULL, 0x40000210000500ULL,
        0x800040100020000ULL, 0x100001002000400ULL, 0x200002000401000ULL, 0x4000800010000200ULL,
        0x8080008000400010ULL, 0x1010004000200004ULL, 0x2020001000080002ULL, 0x4040000400010001ULL,
        0x8080002000040001ULL, 0x1010000800020001ULL, 0x2020000400008001ULL, 0x4040000100004001ULL,
        0x8080001000020001ULL, 0x1010000400008001ULL, 0x2020000200004001ULL, 0x4040000080002001ULL,
        0x8080000800010001ULL, 0x1010000200004001ULL, 0x2020000080002001ULL, 0x4040000040001001ULL,
        0x8080000400008001ULL, 0x1010000080002001ULL, 0x2020000040001001ULL, 0x4040000020000801ULL,
        0x8080000200004001ULL, 0x1010000040001001ULL, 0x2020000020000801ULL, 0x4040000010000401ULL,
        0x8000808000100004ULL, 0x1000101000040002ULL, 0x2000082000020001ULL, 0x4000044000010001ULL,
        0x8000028000008001ULL, 0x1000014000004001ULL, 0x200000a000002001ULL, 0x4000005000001001ULL,
        0x8000002800000801ULL, 0x1000001400000401ULL, 0x2000000a00000201ULL, 0x4000000500000101ULL,
        0x8000000280000081ULL, 0x1000000140000041ULL, 0x20000000a0000021ULL, 0x4000000050000011ULL,
        0x8000000028000081ULL, 0x1000000014000041ULL, 0x200000000a000021ULL, 0x4000000005000011ULL,
        0x8000000002800081ULL, 0x1000000001400041ULL, 0x2000000000a00021ULL, 0x4000000000500011ULL,
        0x8000000000280081ULL, 0x1000000000140041ULL, 0x20000000000a0021ULL, 0x4000000000050011ULL,
        0x8000000000028081ULL, 0x1000000000014041ULL, 0x200000000000a021ULL, 0x4000000000005011ULL
    };

    const Bitboard BISHOP_MAGICS[64] = {
        0x40040844404000ULL, 0x2004208880400ULL, 0x1002104440400ULL, 0x8010820880400ULL,
        0x40040844404000ULL, 0x2004208880400ULL, 0x1002104440400ULL, 0x8010820880400ULL,
        0x40040844404000ULL, 0x2004208880400ULL, 0x1002104440400ULL, 0x8010820880400ULL,
        0x40040844404000ULL, 0x2004208880400ULL, 0x1002104440400ULL, 0x8010820880400ULL,
        0x40040844404000ULL, 0x2004208880400ULL, 0x1002104440400ULL, 0x8010820880400ULL,
        0x40040844404000ULL, 0x2004208880400ULL, 0x1002104440400ULL, 0x8010820880400ULL,
        0x40040844404000ULL, 0x2004208880400ULL, 0x1002104440400ULL, 0x8010820880400ULL,
        0x40040844404000ULL, 0x2004208880400ULL, 0x1002104440400ULL, 0x8010820880400ULL
    };

    // Rook masks for attack generation
    Bitboard ROOK_MASKS[64];
    Bitboard BISHOP_MASKS[64];

    Bitboard ROOK_ATTACKS[64][4096];
    Bitboard BISHOP_ATTACKS[64][512];

    // Initialize the rook attack tables
    void init() {
        for (int sq = 0; sq < 64; sq++) {
            ROOK_MASKS[sq] = 0ULL;
            int r, f;
            int r_from = sq / 8;
            int f_from = sq % 8;

            for (r = r_from + 1; r <= 6; r++)
                ROOK_MASKS[sq] |= (1ULL << (r * 8 + f_from));
            for (r = r_from - 1; r >= 1; r--)
                ROOK_MASKS[sq] |= (1ULL << (r * 8 + f_from));
            for (f = f_from + 1; f <= 6; f++)
                ROOK_MASKS[sq] |= (1ULL << (r_from * 8 + f));
            for (f = f_from - 1; f >= 1; f--)
                ROOK_MASKS[sq] |= (1ULL << (r_from * 8 + f));

            int bits = popcount(ROOK_MASKS[sq]);
            int size = 1 << bits;

            for (int i = 0; i < size; i++) {
                Bitboard occupancy = 0ULL;
                Bitboard temp_mask = ROOK_MASKS[sq];
                for (int j = 0; j < bits; j++) {
                    int lsb = pop_lsb(temp_mask);
                    if (i & (1 << j)) {
                        occupancy |= (1ULL << lsb);
                    }
                }

                int magic_index = (occupancy * ROOK_MAGICS[sq]) >> (64 - bits);
                Bitboard attacks = 0ULL;

                for (r = r_from + 1; r <= 7; r++) {
                    attacks |= (1ULL << (r * 8 + f_from));
                    if (occupancy & (1ULL << (r * 8 + f_from)))
                        break;
                }
                for (r = r_from - 1; r >= 0; r--) {
                    attacks |= (1ULL << (r * 8 + f_from));
                    if (occupancy & (1ULL << (r * 8 + f_from)))
                        break;
                }
                for (f = f_from + 1; f <= 7; f++) {
                    attacks |= (1ULL << (r_from * 8 + f));
                    if (occupancy & (1ULL << (r_from * 8 + f)))
                        break;
                }
                for (f = f_from - 1; f >= 0; f--) {
                    attacks |= (1ULL << (r_from * 8 + f));
                    if (occupancy & (1ULL << (r_from * 8 + f)))
                        break;
                }
                ROOK_ATTACKS[sq][magic_index] = attacks;
            }
        }

        for (int sq = 0; sq < 64; sq++) {
            BISHOP_MASKS[sq] = 0ULL;
            int r, f;
            int r_from = sq / 8;
            int f_from = sq % 8;

            for (r = r_from + 1, f = f_from + 1; r <= 6 && f <= 6; r++, f++)
                BISHOP_MASKS[sq] |= (1ULL << (r * 8 + f));
            for (r = r_from + 1, f = f_from - 1; r <= 6 && f >= 1; r++, f--)
                BISHOP_MASKS[sq] |= (1ULL << (r * 8 + f));
            for (r = r_from - 1, f = f_from + 1; r >= 1 && f <= 6; r--, f++)
                BISHOP_MASKS[sq] |= (1ULL << (r * 8 + f));
            for (r = r_from - 1, f = f_from - 1; r >= 1 && f >= 1; r--, f--)
                BISHOP_MASKS[sq] |= (1ULL << (r * 8 + f));

            int bits = popcount(BISHOP_MASKS[sq]);
            int size = 1 << bits;

            for (int i = 0; i < size; i++) {
                Bitboard occupancy = 0ULL;
                Bitboard temp_mask = BISHOP_MASKS[sq];
                for (int j = 0; j < bits; j++) {
                    int lsb = pop_lsb(temp_mask);
                    if (i & (1 << j)) {
                        occupancy |= (1ULL << lsb);
                    }
                }

                int magic_index = (occupancy * BISHOP_MAGICS[sq]) >> (64 - bits);
                Bitboard attacks = 0ULL;

                for (r = r_from + 1, f = f_from + 1; r <= 7 && f <= 7; r++, f++) {
                    attacks |= (1ULL << (r * 8 + f));
                    if (occupancy & (1ULL << (r * 8 + f)))
                        break;
                }
                for (r = r_from + 1, f = f_from - 1; r <= 7 && f >= 0; r++, f--) {
                    attacks |= (1ULL << (r * 8 + f));
                    if (occupancy & (1ULL << (r * 8 + f)))
                        break;
                }
                for (r = r_from - 1, f = f_from + 1; r >= 0 && f <= 7; r--, f++) {
                    attacks |= (1ULL << (r * 8 + f));
                    if (occupancy & (1ULL << (r * 8 + f)))
                        break;
                }
                for (r = r_from - 1, f = f_from - 1; r >= 0 && f >= 0; r--, f--) {
                    attacks |= (1ULL << (r * 8 + f));
                    if (occupancy & (1ULL << (r * 8 + f)))
                        break;
                }
                BISHOP_ATTACKS[sq][magic_index] = attacks;
            }
        }
    }
}
