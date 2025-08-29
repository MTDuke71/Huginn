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

// Helper function to update castling rights efficiently
constexpr inline uint8_t update_castling_rights_for_squares(uint8_t current_rights, int from_square, int to_square) {
    // Use bitwise operations to efficiently clear castling rights based on square involvement
    uint8_t new_rights = current_rights;
    
    // Clear castling rights when pieces move from or to key squares
    // A1 (21): Clear White Queenside
    if (from_square == 21 || to_square == 21) {
        new_rights &= ~CASTLE_WQ;
    }
    // E1 (25): Clear both White castling rights (king move)
    if (from_square == 25 || to_square == 25) {
        new_rights &= ~(CASTLE_WK | CASTLE_WQ);
    }
    // H1 (28): Clear White Kingside
    if (from_square == 28 || to_square == 28) {
        new_rights &= ~CASTLE_WK;
    }
    // A8 (91): Clear Black Queenside
    if (from_square == 91 || to_square == 91) {
        new_rights &= ~CASTLE_BQ;
    }
    // E8 (95): Clear both Black castling rights (king move)
    if (from_square == 95 || to_square == 95) {
        new_rights &= ~(CASTLE_BK | CASTLE_BQ);
    }
    // H8 (98): Clear Black Kingside
    if (from_square == 98 || to_square == 98) {
        new_rights &= ~CASTLE_BK;
    }
    
    return new_rights;
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

// ---------- Castling Rights Optimization ----------
// Lookup table for efficient castling rights updates during move making
// Each square maps to a mask that preserves only the castling rights that should remain
// when a piece moves from or to that square
namespace CastlingLookup {
    // Initialize castling permission lookup table
    constexpr std::array<uint8_t, 120> create_castling_mask_table() {
        std::array<uint8_t, 120> table{};
        
        // Default: all castling rights preserved (15 = 0b1111)
        for (size_t i = 0; i < 120; ++i) {
            table[i] = CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ;
        }
        
        // Critical squares that affect castling rights:
        table[21] = CASTLE_WK | CASTLE_BK | CASTLE_BQ;  // a1 - clear white queenside
        table[25] = CASTLE_BK | CASTLE_BQ;              // e1 - clear both white rights  
        table[28] = CASTLE_WQ | CASTLE_BK | CASTLE_BQ;  // h1 - clear white kingside
        table[91] = CASTLE_WK | CASTLE_WQ | CASTLE_BK;  // a8 - clear black queenside
        table[95] = CASTLE_WK | CASTLE_WQ;              // e8 - clear both black rights
        table[98] = CASTLE_WK | CASTLE_WQ | CASTLE_BQ;  // h8 - clear black kingside
        
        return table;
    }
    
    // Compile-time generated lookup table
    constexpr auto CASTLING_MASK = create_castling_mask_table();
    
    // Fast castling rights update: AND with masks for both from and to squares
    constexpr inline uint8_t update_castling_rights(uint8_t current_rights, int from_sq, int to_sq) {
        return current_rights & CASTLING_MASK[from_sq] & CASTLING_MASK[to_sq];
    }
}
