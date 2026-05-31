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
            list.moves[list.count++] = move;
        }
    }
}
