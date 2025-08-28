// chess_types.hpp
#pragma once
#include <cstdint>
#include <array>
#include <cassert>
#include <iostream>

#ifdef _MSC_VER
#include "msvc_optimizations.hpp"
#endif

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

// ---------- Castling Permission Array (Performance Optimization) ----------
// CastlePerm[120] - Optimized castling rights update using single array lookup
// Instead of multiple conditional checks, use: castling_rights &= CastlePerm[from_square]
// Values are bitwise masks that clear appropriate castling rights when pieces move from key squares:
// - Most squares: 15 (1111 binary) = keep all castling rights
// - a1 (21): 13 (1101 binary) = clear CASTLE_WQ (white queenside)
// - e1 (25): 12 (1100 binary) = clear CASTLE_WK | CASTLE_WQ (both white rights)
// - h1 (28): 14 (1110 binary) = clear CASTLE_WK (white kingside)
// - a8 (91): 7  (0111 binary) = clear CASTLE_BQ (black queenside)
// - e8 (95): 3  (0011 binary) = clear CASTLE_BK | CASTLE_BQ (both black rights)
// - h8 (98): 11 (1011 binary) = clear CASTLE_BK (black kingside)

#ifdef _MSC_VER
#pragma data_seg(".rdata") // Place in read-only section for better cache locality
#endif

constexpr std::array<uint8_t, 120> CastlePerm = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  // 0-9 (offboard)
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  // 10-19 (offboard)
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,  // 20-29 (rank 1: a1=21→13, e1=25→12, h1=28→14)
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  // 30-39 (rank 2)
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  // 40-49 (rank 3)
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  // 50-59 (rank 4)
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  // 60-69 (rank 5)
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  // 70-79 (rank 6)
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  // 80-89 (rank 7)
    15,  7, 15, 15, 15,  3, 15, 15, 11, 15,  // 90-99 (rank 8: a8=91→7, e8=95→3, h8=98→11)
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  // 100-109 (offboard)
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15   // 110-119 (offboard)
};

#ifdef _MSC_VER
#pragma data_seg() // Reset to default section
#endif

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
    Offboard   = 255,  // Sentinel value for offboard squares in mailbox-120
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
constexpr inline bool is_offboard(Piece p) { return p == Piece::Offboard; }

constexpr inline PieceType type_of(Piece p) {
    return (is_none(p) || is_offboard(p)) ? PieceType::None : PieceType(uint8_t(p) & 0b111);
}
constexpr inline Color color_of(Piece p) {
    return (is_none(p) || is_offboard(p)) ? Color::None : Color((uint8_t(p) >> 3) & 0b1);
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

// ---------- Char ↔ piece type (for FEN) - declarations moved to .cpp ----------
char to_char(Piece p);
Piece from_char(char ch);

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
