/**
 * @file see.cpp
 * @brief SEE implementation. See see.hpp for what this is and how it's used.
 */

#include "see.hpp"
#include "position.hpp"
#include "bitboard.hpp"
#include "square.hpp"
#include "attack_tables.hpp"

#include <algorithm>
#include <cstdint>

namespace Huginn {

namespace {

// Identify the piece type sitting on a single-bit-bitboard `bit`. Caller
// guarantees that bit is occupied (i.e. occupied_bitboard & bit != 0).
inline PieceType piece_type_on(const Position& pos, uint64_t bit) {
    for (int c = 0; c < 2; ++c) {
        if ((pos.color_bitboards[c] & bit) == 0) continue;
        for (int t = int(PieceType::Pawn); t <= int(PieceType::King); ++t) {
            if (pos.piece_bitboards[c][t] & bit) return PieceType(t);
        }
        break; // colour matched but no piece type? shouldn't happen
    }
    return PieceType::None;
}

// Bitboard of all attackers (any colour) attacking `sq64` given an
// arbitrary occupancy `occ`. Mirrors SqAttackedBB but returns the full
// attacker bitboard — needed so SEE can iterate attacker by attacker.
inline uint64_t attackers_to(const Position& pos, int sq64, uint64_t occ) {
    uint64_t attackers = 0;
    constexpr int W = int(Color::White);
    constexpr int B = int(Color::Black);

    // Pawns: a black pawn on `sq64` attacks the squares listed in
    // pawn_attacks[Black][sq64] (its forward diagonals); so a black pawn
    // ATTACKS sq64 from squares where a notional white pawn on sq64 would
    // attack (and vice versa). pawn_attacks[c][sq] = attack squares OF a
    // pawn of colour c sitting on sq, so for "who attacks sq", swap colour.
    attackers |= pawn_attacks[B][sq64] & pos.piece_bitboards[W][int(PieceType::Pawn)];
    attackers |= pawn_attacks[W][sq64] & pos.piece_bitboards[B][int(PieceType::Pawn)];

    // Knights
    uint64_t knights = pos.piece_bitboards[W][int(PieceType::Knight)] |
                       pos.piece_bitboards[B][int(PieceType::Knight)];
    attackers |= knight_attacks[sq64] & knights;

    // Kings
    uint64_t kings = pos.piece_bitboards[W][int(PieceType::King)] |
                     pos.piece_bitboards[B][int(PieceType::King)];
    attackers |= king_attacks[sq64] & kings;

    // Rooks + Queens (rank/file sliders), under given occupancy
    uint64_t rq = pos.piece_bitboards[W][int(PieceType::Rook)]   | pos.piece_bitboards[W][int(PieceType::Queen)] |
                  pos.piece_bitboards[B][int(PieceType::Rook)]   | pos.piece_bitboards[B][int(PieceType::Queen)];
    attackers |= rook_attacks(sq64, occ) & rq;

    // Bishops + Queens (diagonal sliders)
    uint64_t bq = pos.piece_bitboards[W][int(PieceType::Bishop)] | pos.piece_bitboards[W][int(PieceType::Queen)] |
                  pos.piece_bitboards[B][int(PieceType::Bishop)] | pos.piece_bitboards[B][int(PieceType::Queen)];
    attackers |= bishop_attacks(sq64, occ) & bq;

    return attackers;
}

// Pick the least-valuable attacker for `side` from `side_attackers` (which
// must already be filtered to attackers of that colour). Returns the bit
// of the chosen attacker (0 if none) and writes its piece type via out_pt.
inline uint64_t least_valuable_attacker(const Position& pos,
                                        uint64_t side_attackers,
                                        Color side,
                                        PieceType& out_pt) {
    int c = int(side);
    static constexpr PieceType kOrder[6] = {
        PieceType::Pawn, PieceType::Knight, PieceType::Bishop,
        PieceType::Rook, PieceType::Queen,  PieceType::King
    };
    for (PieceType pt : kOrder) {
        uint64_t pieces = side_attackers & pos.piece_bitboards[c][int(pt)];
        if (pieces) {
            out_pt = pt;
            return pieces & (uint64_t)(-(int64_t)pieces); // isolate lsb
        }
    }
    out_pt = PieceType::None;
    return 0;
}

} // namespace

int see(const Position& pos, const S_MOVE& move) {
    int from64 = move.get_from();  // S_MOVE now stores 64-square indices
    int to64   = move.get_to();

    int gain[32];
    int d = 0;

    uint64_t occ = pos.occupied_bitboard;
    uint64_t from_bit = 1ULL << from64;
    uint64_t to_bit   = 1ULL << to64;

    // gain[0] = value of the piece initially captured on the to-square.
    if (move.is_en_passant()) {
        gain[0] = SEE_PIECE_VALUE[int(PieceType::Pawn)];
    } else if (occ & to_bit) {
        gain[0] = SEE_PIECE_VALUE[int(piece_type_on(pos, to_bit))];
    } else {
        gain[0] = 0; // not actually a capture (caller error)
    }

    // The piece that performed the initial capture (the candidate move's
    // attacker). For a promotion-capture, it becomes the promo piece and
    // we add the (promo - pawn) value to gain[0].
    PieceType attacker_pt = piece_type_on(pos, from_bit);
    if (move.is_promotion()) {
        PieceType promo = move.get_promoted();
        gain[0] += SEE_PIECE_VALUE[int(promo)] - SEE_PIECE_VALUE[int(PieceType::Pawn)];
        attacker_pt = promo;
    }

    // Remove the attacker from occupancy.
    occ ^= from_bit;

    // For en passant, also remove the captured pawn from the board.
    if (move.is_en_passant()) {
        int cap_sq = (pos.side_to_move == Color::White) ? to64 - 8 : to64 + 8;
        occ ^= (1ULL << cap_sq);
    }

    uint64_t all_attackers = attackers_to(pos, to64, occ);
    Color side = (pos.side_to_move == Color::White) ? Color::Black : Color::White;

    // Constant slider bitboards (used to re-derive x-ray attackers when a
    // sliding piece is removed from the chain).
    const uint64_t rq_const =
        pos.piece_bitboards[0][int(PieceType::Rook)]   | pos.piece_bitboards[0][int(PieceType::Queen)] |
        pos.piece_bitboards[1][int(PieceType::Rook)]   | pos.piece_bitboards[1][int(PieceType::Queen)];
    const uint64_t bq_const =
        pos.piece_bitboards[0][int(PieceType::Bishop)] | pos.piece_bitboards[0][int(PieceType::Queen)] |
        pos.piece_bitboards[1][int(PieceType::Bishop)] | pos.piece_bitboards[1][int(PieceType::Queen)];

    while (true) {
        // Restrict to currently-on-board attackers
        all_attackers &= occ;
        uint64_t side_attackers = all_attackers & pos.color_bitboards[int(side)];
        PieceType next_pt;
        uint64_t lva_bit = least_valuable_attacker(pos, side_attackers, side, next_pt);
        if (lva_bit == 0) break;

        ++d;
        // gain[d] from the new mover's perspective: capture the piece left
        // on `to` by the previous mover, then expose ourselves to recapture.
        gain[d] = SEE_PIECE_VALUE[int(attacker_pt)] - gain[d - 1];

        // Provable-loss pruning: if the best-case outcome is still bad
        // for the side considering this swap, abandon further depth.
        if (std::max(-gain[d - 1], gain[d]) < 0) break;

        attacker_pt = next_pt;
        occ ^= lva_bit;

        // Re-add x-ray sliding attackers exposed by removing this piece.
        all_attackers |= (rook_attacks(to64, occ) & rq_const) |
                         (bishop_attacks(to64, occ) & bq_const);

        side = (side == Color::White) ? Color::Black : Color::White;
    }

    // Minimax up the gain stack: at each ply, the side-to-capture chooses
    // the better of (take, gain[d]) vs (don't take, -gain[d-1] from above).
    while (d > 0) {
        --d;
        gain[d] = -std::max(-gain[d], gain[d + 1]);
    }
    return gain[0];
}

} // namespace Huginn
