/**
 * @file movegen.cpp
 * @brief Move generation entry points for the Huginn chess engine
 *
 * Thin wrapper around BitboardMoveGen plus the legality and capture filters
 * used by search and quiescence.
 */
#include "movegen.hpp"
#include "position.hpp"
#include "move.hpp"
#include "movegen_bb.hpp"

void generate_all_moves(const Position& pos, S_MOVELIST& list) {
    BitboardMoveGen::generate_all_moves_bitboard(pos, list);
}

// Legal move generation: pseudo-legal + filter by MakeMove legality.
// MakeMove is balanced with TakeMove (and self-undoes on illegal moves
// returning 0), so the filter can run on `pos` directly without copying
// the entire Position. Net change is zero on `pos` after the loop.
//
// #61: copies each surviving move VERBATIM (add_scored_move), preserving the
// generator's scores. The old body re-ran add_capture_move AFTER MakeMove —
// scoring against a board where the source square is already empty and the
// victim gone — and add_quiet_move reset promotion scores to a flat value.
void generate_legal_moves(Position& pos, S_MOVELIST& list) {
    S_MOVELIST pseudo_moves;
    generate_all_moves(pos, pseudo_moves);

    list.count = 0;
    for (int i = 0; i < pseudo_moves.size(); ++i) {
        if (pos.MakeMove(pseudo_moves[i]) == 1) {
            pos.TakeMove();
            list.add_scored_move(pseudo_moves[i]);
        }
    }
}

// Pseudo-legal captures only — no MakeMove/Unmake. Caller is responsible
// for legality (qsearch already does `if (pos.MakeMove(m) != 1) continue`).
// Preserves the pre-move MVV-LVA score set by the bitboard generator.
void generate_all_caps_pseudo(const Position& pos, S_MOVELIST& list) {
    S_MOVELIST all_moves;
    generate_all_moves(pos, all_moves);

    list.count = 0;
    for (int i = 0; i < all_moves.size(); ++i) {
        const S_MOVE& move = all_moves[i];
        if (move.is_capture()) {
            list.add_scored_move(move);
        }
    }
}

// Pseudo-legal tactical frontier for quiescence (BACKLOG #52): captures PLUS
// quiet promotions. A non-capturing promotion changes material as much as
// winning a minor piece — leaving it out of qsearch made the horizon blind to
// it. Same lazy-legality contract as generate_all_caps_pseudo.
void generate_tactical_pseudo(const Position& pos, S_MOVELIST& list) {
    S_MOVELIST all_moves;
    generate_all_moves(pos, all_moves);

    list.count = 0;
    for (int i = 0; i < all_moves.size(); ++i) {
        const S_MOVE& move = all_moves[i];
        if (move.is_capture() || move.is_promotion()) {
            list.add_scored_move(move);
        }
    }
}
