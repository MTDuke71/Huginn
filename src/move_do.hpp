#pragma once
#include <cstdint>
#include "board_state.hpp"  // S_BOARD, piece codes, NO_SQ, WKCA...
#include "board120.hpp"     // sq(), NORTH/SOUTH, file_of()
#include "zobrist.hpp"      // Zobrist tables & compute()

// Per-move saved info so unmake is O(1) and lossless.
struct State {
    int  enPasPrev{NO_SQ};
    int  castlePrev{0};
    int  fiftyPrev{0};
    int  sidePrev{WHITE};
    int  capturedPc{EMPTY};    // piece code captured (or EMPTY)
    std::uint64_t posKeyPrev{0};
};

// Update castle rights bitmask after a move/capture.
// (Declaration; definition is in move_do.cpp)
int updated_castle_perm_after(const S_BOARD& b, int from, int to, int capturedPc);

// Make/unmake with incremental Zobrist updates.
// Flags come from your move decoder (castle/ep/double-pawn-push).
bool make_move(
    S_BOARD& b,
    int from, int to,
    int promoPc,              // 0 or EMPTY if no promotion, else wQ/bN/etc.
    bool isCastle,
    bool isEnPassant,
    bool isDoublePush,
    State& st
);

void unmake_move(
    S_BOARD& b,
    int from, int to,
    int promoPc,
    bool isCastle,
    bool isEnPassant,
    bool isDoublePush,
    const State& st
);
