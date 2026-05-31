/**
 * @file chess_types.hpp
 * @brief Core chess data types and fundamental constants
 * 
 * Defines the fundamental data types used throughout the Huginn chess engine,
 * including pieces, colors, squares, castling rights, and move types. These types
 * form the foundation of the engine's type system and are designed for both
 * performance and type safety.
 * 
 * ## Type System Design
 * - **Strongly Typed Enums**: Prevent implicit conversions and bugs
 * - **Compact Representation**: Memory-efficient storage for search performance
 * - **Fast Operations**: Bitwise operations and lookup tables where possible
 * - **Debug Support**: Comprehensive assertions and validation in debug builds
 * 
 * ## Key Types
 * - **Color**: White/Black/None enumeration
 * - **PieceType**: None, Pawn, Knight, Bishop, Rook, Queen, King, Offboard
 * - **Piece**: Combined color and piece type with fast access macros
 * - **Square**: 64-square board coordinates
 * - **Castling**: Bit flags for efficient castling rights management
 * 
 * ## Performance Features
 * - Constexpr functions for compile-time evaluation
 * - Inline operations for zero-overhead abstractions  
 * - MSVC-specific optimizations when available
 * - Cache-friendly data layout and access patterns
 * 
 * @author MTDuke71
 * @version 1.2
 * @see square.hpp for coordinate system details
 */
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
// Evaluation material values — this is the tunable "truth" of piece worth
// and is expected to DRIFT under Texel/SPSA tuning. Two other subsystems
// have their own independent value tables that merely START at the same
// numbers: SEE_PIECE_VALUE (see.hpp) and the MVV-LVA table (Engine::
// init_mvv_lva). They are NOT meant to track this array — do not merge them
// into one shared constant; coupling capture pruning/ordering to eval
// tuning would inject noise and break MVV-LVA's king=0 special case. The
// matching values are a coincidental starting point, not a constraint.
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
// Lookup table for efficient castling rights updates during move making.
// Each sq64 maps to a mask that preserves only the castling rights that
// should remain when a piece moves from or to that square.
//
// History: a parallel [120] table (CASTLING_MASK / update_castling_rights)
// existed during the S_MOVE 120->64 migration; it lost its last caller
// once MakeMove went 64-native and was deleted with the other dead
// mailbox-120 structures. Only the sq64 table below remains.
namespace CastlingLookup {
    // 64-square table (a1=0, e1=4, h1=7, a8=56, e8=60, h8=63).
    constexpr std::array<uint8_t, 64> create_castling_mask_table_sq64() {
        std::array<uint8_t, 64> table{};
        for (size_t i = 0; i < 64; ++i) {
            table[i] = CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ;
        }
        table[0]  = CASTLE_WK | CASTLE_BK | CASTLE_BQ;  // a1 - clear white queenside
        table[4]  = CASTLE_BK | CASTLE_BQ;              // e1 - clear both white rights
        table[7]  = CASTLE_WQ | CASTLE_BK | CASTLE_BQ;  // h1 - clear white kingside
        table[56] = CASTLE_WK | CASTLE_WQ | CASTLE_BK;  // a8 - clear black queenside
        table[60] = CASTLE_WK | CASTLE_WQ;              // e8 - clear both black rights
        table[63] = CASTLE_WK | CASTLE_WQ | CASTLE_BQ;  // h8 - clear black kingside
        return table;
    }

    constexpr auto CASTLING_MASK_SQ64 = create_castling_mask_table_sq64();

    constexpr inline uint8_t update_castling_rights_sq64(uint8_t current_rights, int from_sq64, int to_sq64) {
        return current_rights & CASTLING_MASK_SQ64[from_sq64] & CASTLING_MASK_SQ64[to_sq64];
    }
}

// ---------- Piece Utility Functions ----------
// Extract piece type from colored piece
constexpr inline PieceType get_piece_type(Piece piece) {
    return PieceType(uint8_t(piece) & 0x07);  // Lower 3 bits contain type
}

// Extract color from colored piece
constexpr inline Color get_piece_color(Piece piece) {
    return Color((uint8_t(piece) >> 3) & 0x01);  // Bit 3 contains color
}

// Check if square is valid (on-board)
constexpr inline bool is_valid_square(int square) {
    return square >= 0 && square < 120 && 
           (square % 10) >= 1 && (square % 10) <= 8 &&
           (square / 10) >= 2 && (square / 10) <= 9;
}
