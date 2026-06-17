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
/// @brief Side to move / piece colour. `None` is the empty/unset sentinel.
enum class Color : uint8_t { White = 0, Black = 1, None = 2 };

/// @brief Opposite colour (`!White == Black`, `!Black == White`, `!None == None`).
constexpr inline Color operator!(Color c) {
    return (c == Color::White) ? Color::Black
         : (c == Color::Black) ? Color::White
                               : Color::None;
}

// ---------- Castling Rights ----------
// Bit flags for castling permissions; OR them together, AND with a mask to clear.
constexpr uint8_t CASTLE_NONE = 0;     ///< No castling rights.
constexpr uint8_t CASTLE_WK   = 1;     ///< White king-side (O-O).
constexpr uint8_t CASTLE_WQ   = 2;     ///< White queen-side (O-O-O).
constexpr uint8_t CASTLE_BK   = 4;     ///< Black king-side (O-O).
constexpr uint8_t CASTLE_BQ   = 8;     ///< Black queen-side (O-O-O).
constexpr uint8_t CASTLE_ALL  = 15;    ///< All four rights (KQkq).

// Legacy aliases (VICE naming) kept for compatibility.
constexpr int WKCA = CASTLE_WK;  ///< @deprecated Alias of CASTLE_WK.
constexpr int WQCA = CASTLE_WQ;  ///< @deprecated Alias of CASTLE_WQ.
constexpr int BKCA = CASTLE_BK;  ///< @deprecated Alias of CASTLE_BK.
constexpr int BQCA = CASTLE_BQ;  ///< @deprecated Alias of CASTLE_BQ.

// ---------- Piece Types (colorless) ----------
/// @brief Colourless piece type. `None` = empty; `_Count` (7) is the array bound,
///        not a real piece. Values double as indices into per-type tables.
enum class PieceType : uint8_t {
    None  = 0,  ///< Empty square / no piece.
    Pawn  = 1,
    Knight= 2,
    Bishop= 3,
    Rook  = 4,
    Queen = 5,
    King  = 6,
    _Count      ///< Number of entries (array-size sentinel).
};

// ---------- Piece List Constants (legacy) ----------
constexpr int MAX_PIECES_PER_TYPE = 10;               ///< Max pieces of one type per side (with promotions).
constexpr int MAX_PIECE_TYPES = int(PieceType::_Count); ///< Number of piece-type slots.

/// @brief Legacy piece-location table (`pList[type][i] = square`, -1 = unused).
/// @deprecated From the mailbox era; the pure-bitboard engine derives piece
///             location from the per-piece bitboards instead.
using PieceList = std::array<std::array<int, MAX_PIECES_PER_TYPE>, MAX_PIECE_TYPES>;

// ---------- Colored Pieces (packed: color<<3 | type) ----------
/// @brief A coloured piece, packed as `(color << 3) | type`. `None` = empty;
///        `Offboard` is a mailbox-120 sentinel (legacy). Use type_of() /
///        color_of() / make_piece() rather than unpacking the bits by hand.
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

constexpr inline bool is_none(Piece p) { return p == Piece::None; }       ///< True for the empty-square value.
constexpr inline bool is_offboard(Piece p) { return p == Piece::Offboard; } ///< True for the offboard sentinel.

/// @brief Colourless type of @p p (None for empty/offboard).
constexpr inline PieceType type_of(Piece p) {
    return (is_none(p) || is_offboard(p)) ? PieceType::None : PieceType(uint8_t(p) & 0b111);
}
/// @brief Colour of @p p (None for empty/offboard).
constexpr inline Color color_of(Piece p) {
    return (is_none(p) || is_offboard(p)) ? Color::None : Color((uint8_t(p) >> 3) & 0b1);
}
/// @brief Pack a colour + type into a Piece (None if either operand is None).
constexpr inline Piece make_piece(Color c, PieceType t) {
    if (c == Color::None || t == PieceType::None) return Piece::None;
    return Piece( (uint8_t(c) << 3) | uint8_t(t) );
}

// ---------- Small conveniences ----------
/// @brief True for the sliding pieces (bishop, rook, queen).
constexpr inline bool is_slider(PieceType t) {
    return t == PieceType::Bishop || t == PieceType::Rook || t == PieceType::Queen;
}
constexpr inline bool is_minor(PieceType t) { return t == PieceType::Knight || t == PieceType::Bishop; } ///< Knight or bishop.
constexpr inline bool is_major(PieceType t) { return t == PieceType::Rook   || t == PieceType::Queen;  } ///< Rook or queen.

// ---------- Char <-> piece (for FEN) — definitions in chess_types.cpp ----------
/// @brief FEN letter for a piece (e.g. WhiteKnight -> 'N', BlackPawn -> 'p'); '.' if empty.
char to_char(Piece p);
/// @brief Piece from a FEN letter (uppercase = White, lowercase = Black); Piece::None if unrecognised.
Piece from_char(char ch);

// ---------- Values / indexing helpers ----------
/**
 * @def EVAL_PARAM
 * @brief Storage class for tunable eval parameters (Texel hook, #9).
 *
 * `inline constexpr` for the release engine (constant-folded, zero cost) but a
 * plain mutable `inline` global under `-DHUGINN_TUNING`, so the tuner can
 * overwrite the tables and re-evaluate. The eval source is identical either
 * way — only the storage class differs by build, so there is NO release NPS
 * impact. (value_of() loses constexpr-ness only in the tuning build; its call
 * sites are all runtime anyway.)
 */
#ifdef HUGINN_TUNING
  #define EVAL_PARAM inline
#else
  #define EVAL_PARAM inline constexpr
#endif

/**
 * @brief Midgame material values, indexed by PieceType (Texel-tuned, #9; was
 *        the classic 100/320/330/500/900).
 *
 * This is the tunable "truth" of piece worth and is expected to DRIFT under
 * tuning. Two other subsystems keep their own independent value tables that
 * merely START at the same classic numbers — SEE_PIECE_VALUE (see.hpp) and the
 * MVV-LVA table (Engine::init_mvv_lva). They must NOT be merged into one shared
 * constant: coupling capture pruning/ordering to eval tuning would inject noise
 * and break MVV-LVA's king=0 special case. @see PIECE_VALUES for the fixed
 * ordering/material table.
 */
EVAL_PARAM std::array<int, size_t(PieceType::_Count)> PIECE_VALUES_MG = { 0, 88, 396, 391, 532, 1188, 20000 };

/**
 * @brief Endgame material values, indexed by PieceType (BACKLOG #35 Exp 2;
 *        Texel-tuned, #9). Blended against PIECE_VALUES_MG by game phase in
 *        evaluate() when ENABLE_TAPERED_MATERIAL is set.
 *
 * Standard endgame directions emerged from the fit: pawns up (promotion
 * potential), rook/queen up (open boards), knight down (can't cover both
 * wings), bishop up (long diagonals). EVAL-ONLY — SEE / MVV-LVA keep their own
 * MG-based tables.
 */
EVAL_PARAM std::array<int, size_t(PieceType::_Count)> PIECE_VALUES_EG = { 0, 93, 291, 313, 566, 1065, 20000 };

/**
 * @brief Fixed canonical piece values for move ordering + incremental material
 *        tracking (used by value_of). Deliberately the classic 100/320/.../900,
 *        NOT the tuned PIECE_VALUES_MG — ordering/material must not drift with
 *        eval tuning. A third independent table alongside SEE_PIECE_VALUE and
 *        the MVV-LVA table.
 */
constexpr std::array<int, size_t(PieceType::_Count)> PIECE_VALUES = {
    0, 100, 320, 330, 500, 900, 20000
};

/// @brief Canonical (untuned) material value of a piece; 0 for empty/offboard.
constexpr inline int value_of(Piece p) {
    return PIECE_VALUES[size_t(type_of(p))];
}

/// @brief Range adaptor to iterate the real piece types Pawn..King, skipping
///        None and _Count. Usage: `for (auto t : PieceTypeIter()) { ... }`.
struct PieceTypeIter {
    struct It {
        uint8_t v;
        bool operator!=(const It& o) const { return v != o.v; }
        void operator++() { ++v; }
        PieceType operator*() const { return PieceType(v); }
    };
    It begin() const { return {1}; } ///< Starts at Pawn (skips None).
    It end()   const { return {uint8_t(PieceType::_Count)}; } ///< One past King.
};

// ---------- Castling Rights Optimization ----------
// Lookup table for efficient castling rights updates during move making.
// Each sq64 maps to a mask that preserves only the castling rights that
// should remain when a piece moves from or to that square.
//
// History: a parallel [120] table (CASTLING_MASK / update_castling_rights)
// existed during the S_MOVE 120->64 migration; it lost its last caller
// once MakeMove went 64-native and was deleted with the other dead
// mailbox-120 structures. Only the sq64 table below remains.
/**
 * @namespace CastlingLookup
 * @brief Castling-rights update table.
 *
 * Maps each square (sq64) to a mask that clears exactly the castling rights
 * invalidated when a piece moves from or to it (the king and rook home
 * squares). MakeMove ANDs this mask into the castling state for O(1) updates.
 */
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
/// @brief Raw type bits of a piece (lower 3 bits). Unlike type_of(), does NOT
///        special-case None/Offboard — use type_of() for board squares.
constexpr inline PieceType get_piece_type(Piece piece) {
    return PieceType(uint8_t(piece) & 0x07);  // Lower 3 bits contain type
}

/// @brief Raw colour bit of a piece (bit 3). Unlike color_of(), does NOT
///        special-case None/Offboard — use color_of() for board squares.
constexpr inline Color get_piece_color(Piece piece) {
    return Color((uint8_t(piece) >> 3) & 0x01);  // Bit 3 contains color
}

