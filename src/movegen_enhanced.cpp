/**
 * @file movegen_enhanced.cpp
 * @brief Move generation entry points for the Huginn chess engine
 *
 * Thin wrapper around BitboardMoveGen plus the legality and capture filters
 * used by search and quiescence.
 */
#include "movegen_enhanced.hpp"
#include "position.hpp"
#include "move.hpp"
#include "bitboard_movegen.hpp"

void generate_all_moves(const Position& pos, S_MOVELIST& list) {
    BitboardMoveGen::generate_all_moves_bitboard(pos, list);
}

// Legal move generation: pseudo-legal + filter by MakeMove legality
void generate_legal_moves_enhanced(Position& pos, S_MOVELIST& list) {
    S_MOVELIST pseudo_moves;
    generate_all_moves(pos, pseudo_moves);

    list.count = 0;
    Position temp_pos = pos;
    for (int i = 0; i < pseudo_moves.size(); ++i) {
        if (temp_pos.MakeMove(pseudo_moves[i]) == 1) {
            list.add_quiet_move(pseudo_moves[i]);
            temp_pos.TakeMove();
        }
    }
}

// Quiescence: legal captures only
void generate_all_caps(Position& pos, S_MOVELIST& list) {
    S_MOVELIST all_moves;
    generate_all_moves(pos, all_moves);

    list.count = 0;
    Position temp_pos = pos;
    for (int i = 0; i < all_moves.size(); ++i) {
        S_MOVE move = all_moves[i];
        if (move.is_capture()) {
            if (temp_pos.MakeMove(move) == 1) {
                list.add_capture_move(move, pos);
                temp_pos.TakeMove();
            }
        }
    }
}
