/**
 * @brief Encodes a chess move into an integer representation.
 * 
 * Move encoding bit layout (25 bits total in int move):
 * Bits 0-6:   from square (7 bits, 0-127, supports 120-square notation)
 * Bits 7-13:  to square (7 bits, 0-127, supports 120-square notation)  
 * Bits 14-17: captured piece type (4 bits, 0-15, PieceType enum)
 * Bit 18:     en passant capture flag (1 bit)
 * Bit 19:     pawn start (double push) flag (1 bit)
 * Bits 20-23: promoted piece type (4 bits, 0-15, PieceType enum)
 * Bit 24:     castle move flag (1 bit)
 *
 * This static method constructs a compact integer encoding of a move, including
 * source and destination squares, captured and promoted piece types, and special
 * move flags (en passant, pawn double start, castling).
 *
 * @param from        The source square index (0-127).
 * @param to          The destination square index (0-127).
 * @param captured    The type of piece captured (if any).
 * @param en_passant  True if the move is an en passant capture.
 * @param pawn_start  True if the move is a pawn's initial double-step.
 * @param promoted    The type of piece the pawn is promoted to (if any).
 * @param castle      True if the move is a castling move.
 * @return            Encoded integer representing the move.
 */
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
