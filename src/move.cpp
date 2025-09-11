/**
 * @file move.cpp
 * @brief Implementation of S_MOVE encoding functionality
 * 
 * This file contains the implementation of the static move encoding function
 * that was moved from the header to keep the interface clean and reduce
 * compilation dependencies.
 */

/**
 * @brief Encodes a chess move into a compact 25-bit integer representation
 * 
 * This function combines all move components into a single integer using bitwise
 * operations and predefined bit shifts. The encoding format supports all chess
 * move types including normal moves, captures, promotions, castling, en passant,
 * and pawn double-pushes.
 * 
 * Move encoding bit layout (25 bits total):
 * Bits 0-6:   Source square (7 bits, 0-127, supports 120-square notation)
 * Bits 7-13:  Destination square (7 bits, 0-127, supports 120-square notation)  
 * Bits 14-17: Captured piece type (4 bits, 0-15, PieceType enum values)
 * Bit 18:     En passant capture flag (1 bit)
 * Bit 19:     Pawn start (double push) flag (1 bit)
 * Bits 20-23: Promoted piece type (4 bits, 0-15, PieceType enum values)
 * Bit 24:     Castle move flag (1 bit)
 * 
 * The encoding uses bitwise OR operations to combine all components:
 * - Source/destination squares are masked to 7 bits (supports 0-127)
 * - Piece types are cast to int and masked to 4 bits (supports enum values 0-15)
 * - Boolean flags are converted to their respective bit positions using ternary operators
 * 
 * @param from        Source square index (0-127, typically 0-119 for 120-square board)
 * @param to          Destination square index (0-127, typically 0-119 for 120-square board)
 * @param captured    Type of piece captured (PieceType::None if no capture)
 * @param en_passant  True if the move is an en passant capture
 * @param pawn_start  True if the move is a pawn's initial double-step
 * @param promoted    Type of piece to promote to (PieceType::None if not promotion)
 * @param castle      True if the move is a castling move
 * @return            Encoded 25-bit integer representing the complete move
 * 
 * @complexity        O(1) - constant time bitwise operations
 * @threadsafe        Yes - pure function with no shared state
 * 
 * @example
 * // Create a quiet move from e2 to e4
 * int move = S_MOVE::encode_move(E2, E4);
 * 
 * // Create a capture move
 * int capture = S_MOVE::encode_move(E4, D5, PieceType::Pawn);
 * 
 * // Create a promotion with capture
 * int promotion = S_MOVE::encode_move(E7, D8, PieceType::Queen, false, false, PieceType::Queen);
 */
#include "move.hpp"

// ============================================================================
// S_MOVE IMPLEMENTATION
// Non-inline methods that are too complex for the header file
// ============================================================================

/**
 * @brief Static move encoding implementation (DEPRECATED - use factory functions)
 * 
 * This function is retained for backward compatibility but is less efficient
 * than the constexpr factory functions (make_move, make_capture, etc.) which
 * can be optimized at compile time.
 * 
 * @deprecated Use constexpr factory functions instead for better performance
 * @note This implementation may be removed in future versions
 */
[[deprecated("Use constexpr factory functions like make_move() for better performance")]]
int S_MOVE::encode_move(int from, int to, PieceType captured,
                       bool en_passant, bool pawn_start,
                       PieceType promoted, bool castle) {
    return (from & 0x7F) |                                      // Bits 0-6:   source square (masked to 7 bits)
           ((to & 0x7F) << MOVE_TO_SHIFT) |                     // Bits 7-13:  destination (masked + shifted)
           ((int(captured) & 0xF) << MOVE_CAPTURED_SHIFT) |     // Bits 14-17: captured piece (enum→int, masked + shifted)  
           (en_passant ? MOVE_ENPASSANT : 0) |                  // Bit 18:     en passant flag (conditional bit set)
           (pawn_start ? MOVE_PAWNSTART : 0) |                  // Bit 19:     pawn double-push flag (conditional bit set)
           ((int(promoted) & 0xF) << MOVE_PROMOTED_SHIFT) |     // Bits 20-23: promoted piece (enum→int, masked + shifted)
           (castle ? MOVE_CASTLE : 0);                          // Bit 24:     castle flag (conditional bit set)
}
