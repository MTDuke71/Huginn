// chess_types.hpp
#pragma once
#include <cstdint>
#include <array>
#include <cassert>
#include <iostream>

// ---------- Debug Assertions ----------
#ifdef DEBUG
    #define DEBUG_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                std::cerr << "ASSERTION FAILED: " << (message) \
                         << "\n  File: " << __FILE__ \
                         << "\n  Line: " << __LINE__ \
                         << "\n  Function: " << __FUNCTION__ \
                         << "\n  Condition: " << #condition << std::endl; \
                std::abort(); \
            } \
        } while (0)
#else
    #define DEBUG_ASSERT(condition, message) ((void)0)
#endif

// Convenience macro for simple assertions without custom message
#ifdef DEBUG
    #define CHESS_ASSERT(condition) \
        DEBUG_ASSERT(condition, "Chess engine assertion failed")
#else
    #define CHESS_ASSERT(condition) ((void)0)
#endif

// ---------- Colors ----------
enum class Color : uint8_t { White = 0, Black = 1, None = 2 };

constexpr inline Color operator!(Color c) {
    return (c == Color::White) ? Color::Black
         : (c == Color::Black) ? Color::White
                               : Color::None;
}

// ---------- Castling Rights ----------
// Bit flags for castling permissions (can be combined with |)
constexpr uint8_t CASTLE_NONE = 0;     // No castling rights
constexpr uint8_t CASTLE_WK   = 1;     // White King-side (WKCA)
constexpr uint8_t CASTLE_WQ   = 2;     // White Queen-side (WQCA)  
constexpr uint8_t CASTLE_BK   = 4;     // Black King-side (BKCA)
constexpr uint8_t CASTLE_BQ   = 8;     // Black Queen-side (BQCA)
constexpr uint8_t CASTLE_ALL  = 15;    // All castling rights (KQkq)

// Legacy aliases for compatibility
constexpr int WKCA = CASTLE_WK;
constexpr int WQCA = CASTLE_WQ;
constexpr int BKCA = CASTLE_BK;
constexpr int BQCA = CASTLE_BQ;

// Castling utility functions
constexpr inline bool can_castle_kingside(uint8_t rights, Color c) {
    return c == Color::White ? (rights & CASTLE_WK) : (rights & CASTLE_BK);
}

constexpr inline bool can_castle_queenside(uint8_t rights, Color c) {
    return c == Color::White ? (rights & CASTLE_WQ) : (rights & CASTLE_BQ);
}

constexpr inline bool can_castle(uint8_t rights, Color c) {
    return can_castle_kingside(rights, c) || can_castle_queenside(rights, c);
}

constexpr inline uint8_t remove_castling_rights(uint8_t rights, Color c) {
    return c == Color::White ? (rights & ~(CASTLE_WK | CASTLE_WQ)) 
                             : (rights & ~(CASTLE_BK | CASTLE_BQ));
}

// ---------- Piece Types (colorless) ----------
enum class PieceType : uint8_t {
    None  = 0,
    Pawn  = 1,
    Knight= 2,
    Bishop= 3,
    Rook  = 4,
    Queen = 5,
    King  = 6,
    _Count
};

// ---------- Piece List Constants ----------
constexpr int MAX_PIECES_PER_TYPE = 10;  // Maximum pieces of one type per side
constexpr int MAX_PIECE_TYPES = int(PieceType::_Count);

// 2D array to track piece locations: pList[piece_type][index] = square
// Each entry contains the square number where the piece is located
// -1 indicates no piece at that index
using PieceList = std::array<std::array<int, MAX_PIECES_PER_TYPE>, MAX_PIECE_TYPES>;

// ---------- Colored Pieces (packed: color<<3 | type) ----------
enum class Piece : uint8_t {
    None       = 0,
    WhitePawn  = (uint8_t(Color::White) << 3) | uint8_t(PieceType::Pawn),
    WhiteKnight= (uint8_t(Color::White) << 3) | uint8_t(PieceType::Knight),
    WhiteBishop= (uint8_t(Color::White) << 3) | uint8_t(PieceType::Bishop),
    WhiteRook  = (uint8_t(Color::White) << 3) | uint8_t(PieceType::Rook),
    WhiteQueen = (uint8_t(Color::White) << 3) | uint8_t(PieceType::Queen),
    WhiteKing  = (uint8_t(Color::White) << 3) | uint8_t(PieceType::King),

    BlackPawn  = (uint8_t(Color::Black) << 3) | uint8_t(PieceType::Pawn),
    BlackKnight= (uint8_t(Color::Black) << 3) | uint8_t(PieceType::Knight),
    BlackBishop= (uint8_t(Color::Black) << 3) | uint8_t(PieceType::Bishop),
    BlackRook  = (uint8_t(Color::Black) << 3) | uint8_t(PieceType::Rook),
    BlackQueen = (uint8_t(Color::Black) << 3) | uint8_t(PieceType::Queen),
    BlackKing  = (uint8_t(Color::Black) << 3) | uint8_t(PieceType::King),
};

constexpr inline bool is_none(Piece p) { return p == Piece::None; }

constexpr inline PieceType type_of(Piece p) {
    return is_none(p) ? PieceType::None : PieceType(uint8_t(p) & 0b111);
}
constexpr inline Color color_of(Piece p) {
    return is_none(p) ? Color::None : Color((uint8_t(p) >> 3) & 0b1);
}
constexpr inline Piece make_piece(Color c, PieceType t) {
    if (c == Color::None || t == PieceType::None) return Piece::None;
    return Piece( (uint8_t(c) << 3) | uint8_t(t) );
}

// ---------- Small conveniences ----------
constexpr inline bool is_slider(PieceType t) {
    return t == PieceType::Bishop || t == PieceType::Rook || t == PieceType::Queen;
}
constexpr inline bool is_minor(PieceType t) { return t == PieceType::Knight || t == PieceType::Bishop; }
constexpr inline bool is_major(PieceType t) { return t == PieceType::Rook   || t == PieceType::Queen;  }

// ---------- Char ↔ piece type (for FEN) ----------
constexpr inline char to_char(Piece p) {
    const PieceType t = type_of(p);
    if (t == PieceType::None) return '.';
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

constexpr inline Piece from_char(char ch) {
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

// ---------- Values / indexing helpers ----------
constexpr std::array<int, size_t(PieceType::_Count)> PIECE_VALUES_MG = {
    0,   // None
    100, // Pawn
    320, // Knight
    330, // Bishop
    500, // Rook
    900, // Queen
    20000// King (or very large sentinel)
};

constexpr inline int value_of(Piece p) {
    return PIECE_VALUES_MG[size_t(type_of(p))];
}

// Optional: piece loops
struct PieceTypeIter {
    struct It {
        uint8_t v;
        bool operator!=(const It& o) const { return v != o.v; }
        void operator++() { ++v; }
        PieceType operator*() const { return PieceType(v); }
    };
    It begin() const { return {1}; } // skip None
    It end()   const { return {uint8_t(PieceType::_Count)}; }
};

// Example: for (auto t : PieceTypeIter()) {...}
