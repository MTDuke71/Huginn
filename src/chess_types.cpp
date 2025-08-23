#include "chess_types.hpp"

// Non-performance-critical utility functions moved from header

// Piece character conversion - not performance critical, moved from header
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

// Piece parsing from character - not performance critical, moved from header
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
