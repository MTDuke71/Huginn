/**
 * @file chess_types.cpp
 * @brief Implementation of chess type utility functions
 * 
 * Contains non-performance-critical utility functions that have been moved from
 * headers to reduce compilation overhead. These functions handle piece character
 * conversion, parsing, and other chess-related type operations that are used
 * primarily for I/O and debugging rather than core engine performance.
 * 
 * ## Functions
 * - to_char(): Convert piece to display character
 * - from_char(): Parse piece from character input
 * - Character mapping follows standard algebraic notation
 * 
 * @author MTDuke71
 * @version 1.2
 * @see chess_types.hpp for core type definitions
 */
#include "chess_types.hpp"

// Non-performance-critical utility functions moved from header

/**
 * @brief Convert a piece to its character representation
 * 
 * Converts chess pieces to single characters following standard algebraic notation.
 * White pieces are uppercase, black pieces are lowercase. Special characters for
 * empty squares and off-board squares are also handled.
 * 
 * @param p The piece to convert
 * @return Character representation:
 *         - White pieces: K, Q, R, B, N, P
 *         - Black pieces: k, q, r, b, n, p
 *         - Empty squares: '.'
 *         - Off-board squares: '#'
 *         - Invalid pieces: '?'
 */
char to_char(Piece p) {
    const PieceType t = type_of(p);
    if (t == PieceType::None) {
        if (is_offboard(p)) return '#';  // Offboard squares shown as '#'
        return '.';  // Empty squares shown as '.'
    }
    const bool white = (color_of(p) == Color::White);
    char c = '?';
    switch (t) {
        case PieceType::Pawn:   c = 'p'; break;
        case PieceType::Knight: c = 'n'; break;
        case PieceType::Bishop: c = 'b'; break;
        case PieceType::Rook:   c = 'r'; break;
        case PieceType::Queen:  c = 'q'; break;
        case PieceType::King:   c = 'k'; break;
        default:                c = '?'; break;
    }
    return white ? char(c - 'a' + 'A') : c; // upper for White, lower for Black
}

/**
 * @brief Parse a piece from its character representation
 * 
 * Converts character input to chess piece objects. Handles both uppercase (White)
 * and lowercase (Black) characters following standard algebraic notation.
 * 
 * @param ch Character to parse (K/k, Q/q, R/r, B/b, N/n, P/p)
 * @return Corresponding Piece object, or empty piece for invalid input
 */
Piece from_char(char ch) {
    Color c = (ch >= 'A' && ch <= 'Z') ? Color::White :
              (ch >= 'a' && ch <= 'z') ? Color::Black : Color::None;
    char lc = (ch >= 'A' && ch <= 'Z') ? char(ch - 'A' + 'a') : ch;

    PieceType t = PieceType::None;
    switch (lc) {
        case 'p': t = PieceType::Pawn;   break;
        case 'n': t = PieceType::Knight; break;
        case 'b': t = PieceType::Bishop; break;
        case 'r': t = PieceType::Rook;   break;
        case 'q': t = PieceType::Queen;  break;
        case 'k': t = PieceType::King;   break;
        default:  t = PieceType::None;   break;
    }
    return make_piece(c, t);
}
