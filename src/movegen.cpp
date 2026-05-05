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
// Also preserves capture-vs-quiet classification so MVV-LVA scoring set
// by the bitboard generator survives the filter (was previously clobbered
// by `add_quiet_move` for everything).
void generate_legal_moves(Position& pos, S_MOVELIST& list) {
    S_MOVELIST pseudo_moves;
    generate_all_moves(pos, pseudo_moves);

    list.count = 0;
    for (int i = 0; i < pseudo_moves.size(); ++i) {
        if (pos.MakeMove(pseudo_moves[i]) == 1) {
            if (pseudo_moves[i].is_capture()) {
                list.add_capture_move(pseudo_moves[i], pos);
            } else {
                list.add_quiet_move(pseudo_moves[i]);
            }
            pos.TakeMove();
        }
    }
}

// Quiescence: legal captures only.
// MakeMove is balanced with TakeMove and self-undoes when returning 0
// (illegal), so we can run the legality filter on `pos` directly and
// skip the cost of copying the entire Position. This keeps qsearch's
// "SEE before MakeMove" code path correct (callers see the pre-move
// pos when SEE-pruning).
void generate_all_caps(Position& pos, S_MOVELIST& list) {
    S_MOVELIST all_moves;
    generate_all_moves(pos, all_moves);

    list.count = 0;
    for (int i = 0; i < all_moves.size(); ++i) {
        const S_MOVE& move = all_moves[i];
        if (move.is_capture()) {
            if (pos.MakeMove(move) == 1) {
                list.add_capture_move(move, pos);
                pos.TakeMove();
            }
        }
    }
}
