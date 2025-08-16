#pragma once
#include <cstdint>
#include "position.hpp"  // Use Position struct
#include "board120.hpp"     // sq(), NORTH/SOUTH, file_of()
#include "zobrist.hpp"      // Zobrist tables & compute()

// State struct is defined in position.hpp; do not redefine here.

// Update castle rights bitmask after a move/capture.
// (Declaration; definition is in move_do.cpp)
int updated_castle_perm_after(const Position& b, int from, int to, Piece captured);

// Make/unmake with incremental Zobrist updates.
// Flags come from your move decoder (castle/ep/double-pawn-push).
bool make_move(
    Position& b,
    int from, int to,
    Piece promo,
    bool isCastle,
    bool isEnPassant,
    bool isDoublePush,
    State& st
);

void unmake_move(
    Position& b,
    int from, int to,
    Piece promo,
    bool isCastle,
    bool isEnPassant,
    bool isDoublePush,
    const State& st
);
