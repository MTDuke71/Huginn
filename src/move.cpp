#include "move.hpp"

// Non-trivial S_MOVE methods that don't need to be inline

// Static move encoding function (moved from header for cleaner interface)
int S_MOVE::encode_move(int from, int to, PieceType captured,
                       bool en_passant, bool pawn_start,
                       PieceType promoted, bool castle) {
    return (from & 0x7F) |
           ((to & 0x7F) << MOVE_TO_SHIFT) |
           ((int(captured) & 0xF) << MOVE_CAPTURED_SHIFT) |
           (en_passant ? MOVE_ENPASSANT : 0) |
           (pawn_start ? MOVE_PAWNSTART : 0) |
           ((int(promoted) & 0xF) << MOVE_PROMOTED_SHIFT) |
           (castle ? MOVE_CASTLE : 0);
}
