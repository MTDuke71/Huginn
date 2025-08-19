#pragma once
#include <vector>
#include <algorithm>
#include "position.hpp"
#include "move.hpp"
#include "board120.hpp"
#include "movegen_simple.hpp"
#include "sq_attacked.hpp"

// For now, treat pseudo-legal==legal until check logic is added.
inline void generate_legal_moves(const Position& pos, MoveList& out) {
    generate_legal_moves_simple(pos, out);
}

inline bool is_legal_move(const Position& pos, const S_MOVE& move) {
    Position temp_pos = pos;
    State st{};
    make_move(temp_pos, move, st);
    return !SqAttacked(temp_pos.king_sq[static_cast<int>(pos.side_to_move)], temp_pos, !pos.side_to_move);
}
