#include "move.hpp"

// Non-trivial S_MOVE methods that don't need to be inline

// Static decoding function for compatibility
void S_MOVE::decode_move(int encoded, int& from, int& to, PieceType& promo) {
    from = (encoded & MOVE_FROM_MASK) >> MOVE_FROM_SHIFT;
    to = (encoded & MOVE_TO_MASK) >> MOVE_TO_SHIFT;
    promo = PieceType((encoded & MOVE_PROMOTED_MASK) >> MOVE_PROMOTED_SHIFT);
}

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
