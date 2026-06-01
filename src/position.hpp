/**
 * @file position.hpp
 * @brief Chess position representation and manipulation
 * 
 * Implements the core Position class that represents a complete chess position
 * including piece placement, game state, move history, and position evaluation
 * context. The Position class is the central data structure for the Huginn engine,
 * optimized for fast move making/unmaking and efficient position analysis.
 * 
 * ## Position Representation
 * - **Bitboards**: Per-piece bitboards are the single source of truth for
 *   placement, movegen, and evaluation
 * - **sq64 indexing**: squares are 0..63 (a1=0, h8=63)
 * - **Zobrist Hashing**: Incremental hash updates for transposition table
 * 
 * ## State Management
 * - **Game State**: Side to move, castling rights, en passant, halfmove clock
 * - **Move History**: Complete undo information for search tree traversal
 * - **Hash Keys**: Position hashing for repetition detection and TT lookup
 * - **Search Context**: Additional state needed for search algorithms
 * 
 * ## Performance Features
 * - **Incremental Updates**: Fast make/unmake move operations
 * - **Copy-Make Optimization**: Efficient position copying when needed
 * - **Cache-Friendly Layout**: Hot data packed for optimal memory access
 * - **SIMD Integration**: Hardware acceleration for bulk operations
 * 
 * ## Key Operations
 * - Position setup from FEN strings
 * - Legal move generation and validation
 * - Move making and unmaking with full state preservation
 * - Position evaluation and analysis
 * 
 * @author MTDuke71
 * @version 1.2
 * @see chess_types.hpp for fundamental types
 * @see move.hpp for move representation
 */
#pragma once
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "bitboard.hpp"
#include "chess_types.hpp"
#include "move.hpp"
#include "msvc_optimizations.hpp"
#include "zobrist.hpp"

// Forward declarations to avoid circular dependencies
struct S_MOVELIST;

struct State {
    // Minimal saved state for unmake
    int ep_square;            // previous ep square
    uint8_t castling_rights;  // previous castling rights
    uint16_t halfmove_clock;  // previous halfmove clock
    Piece captured;           // captured piece (if any)
};

// Enhanced undo structure for comprehensive move history
struct S_UNDO {
    S_MOVE move;              // Full move information with encoding
    uint8_t castling_rights;  // previous castling permissions (castlePerm)
    int ep_square;            // previous en passant square (enPas)
    uint16_t halfmove_clock;  // previous fifty move counter (fiftyMove)
    uint64_t zobrist_key;     // previous position key (posKey)
    Piece captured;           // captured piece (if any)

    // (king_sq / material_score backups removed: TakeMove restores king_sq
    // directly and material_score is maintained incrementally by the atomic
    // piece ops, so the backups were write-only — see Priority 7.)

    // Constructor
    S_UNDO() : move(), castling_rights(0), ep_square(-1), halfmove_clock(0), zobrist_key(0), captured(Piece::None) {}
};

class Position {
public:
    Color side_to_move{Color::White};
    int ep_square{-1};               // 64-square index (0..63) or -1
    uint8_t castling_rights{0};      // bitmask: CASTLE_WK|CASTLE_WQ|CASTLE_BK|CASTLE_BQ
    uint16_t halfmove_clock{0};
    uint16_t fullmove_number{1};
    std::array<int, 2> king_sq{ -1, -1 }; // [White, Black] king locations (sq64, or -1)
    
    // Full bitboard representation for all piece types [Color][PieceType]
    std::array<std::array<Bitboard, int(PieceType::_Count)>, 2> piece_bitboards{};
    std::array<Bitboard, 2> color_bitboards{ 0, 0 }; // [White, Black] all pieces
    Bitboard occupied_bitboard{ 0 }; // All pieces (White | Black)
    
    uint64_t zobrist_key{0};
    
    // Material score tracking for fast evaluation
    std::array<int, 2> material_score{ 0, 0 }; // [White, Black] material balance
    
    // Move history for undo functionality - use vector for dynamic sizing
    std::vector<S_UNDO> move_history;
    int ply{0};                      // current search/game ply

    // Constructor
    Position() : move_history() {
        move_history.reserve(64);  // Reserve reasonable initial capacity
    }

    void reset();
    bool set_from_fen(const std::string& fen);
    std::string to_fen() const;
    void rebuild_counts();
    void set_startpos();
    
    // VICE Tutorial Video #41: MakeMove function
    int MakeMove(const S_MOVE& move);
    
    // VICE Tutorial Video #42: TakeMove function
    void TakeMove();
    
    // VICE Part 83: Null move functions for null move pruning
    void MakeNullMove();
    void TakeNullMove();
    
    // Update Zobrist key incrementally for a move using XOR (much faster than recomputation)
    void update_zobrist_for_move(const S_MOVE& m, Piece moving, Piece captured, uint8_t old_castling_rights, int old_ep_square);
    
    // Compute and set the Zobrist key from current position
    void update_zobrist_key();

    // Direct 64-square accessor. Caller must guarantee s64 is in [0, 64).
    //
    // History: BACKLOG #26 (e61f6e5) added a board64[64] piece-on-square
    // cache to make this an array load; bench gained +12% NPS but pooled
    // 400g vs t5 came in at -13 Elo (Intel +12 / AMD -38). The invariant
    // test (b8cd310) confirmed the cache was NOT desyncing — the +64
    // bytes of cache footprint cost as much as the loop saved on this
    // codebase. Reverted; bitboard scan kept.
    FORCE_INLINE Piece at_sq64(int s64) const {
        assert(s64 >= 0 && s64 < 64);
        uint64_t bit = 1ULL << s64;
        if ((occupied_bitboard & bit) == 0) return Piece::None;
        int c = (color_bitboards[0] & bit) ? 0 : 1;
        for (int t = int(PieceType::Pawn); t <= int(PieceType::King); ++t) {
            if (piece_bitboards[c][t] & bit) {
                return make_piece(Color(c), PieceType(t));
            }
        }
        return Piece::None;  // unreachable when bitboards are consistent
    }
    // Place / clear a piece at a 64-square index using bitboard storage.
    // If a piece was already at this square it is removed first; passing
    // Piece::None clears the square. Caller guarantees s64 in [0,64).
    inline void set_sq64(int s64, Piece p) {
        uint64_t bit = 1ULL << s64;

        // Clear any existing occupant
        if (occupied_bitboard & bit) {
            int c = (color_bitboards[0] & bit) ? 0 : 1;
            color_bitboards[c] &= ~bit;
            for (int t = int(PieceType::Pawn); t <= int(PieceType::King); ++t) {
                piece_bitboards[c][t] &= ~bit;
            }
            occupied_bitboard &= ~bit;
        }

        // Place new piece (skip None / Offboard)
        if (!is_none(p) && p != Piece::Offboard) {
            int ci = int(color_of(p));
            int ti = int(type_of(p));
            piece_bitboards[ci][ti] |= bit;
            color_bitboards[ci] |= bit;
            occupied_bitboard |= bit;
        }
    }

    // ---- 64-square-native atomic piece ops (sole make/unmake path) --------
    // The 120-square move_piece/clear_piece/add_piece were removed once the
    // S_MOVE 120->64 migration left them with no callers. Caller passes a
    // 64-square index directly; the index must be in [0,64).
    void move_piece_sq64(int from_sq64, int to_sq64) {
        DEBUG_ASSERT(from_sq64 >= 0 && from_sq64 < 64, "Invalid source sq64 for piece move");
        DEBUG_ASSERT(to_sq64 >= 0 && to_sq64 < 64, "Invalid destination sq64 for piece move");

        Piece piece = at_sq64(from_sq64);
        DEBUG_ASSERT(!is_none(piece), "Cannot move piece from empty square");
        DEBUG_ASSERT(is_none(at_sq64(to_sq64)), "Cannot move piece to occupied square");

        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        int zpc = int(piece_type) + (piece_color == Color::Black ? 6 : 0);

        zobrist_key ^= Zobrist::Piece[zpc][from_sq64];
        zobrist_key ^= Zobrist::Piece[zpc][to_sq64];

        popBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], from_sq64);
        popBit(color_bitboards[size_t(piece_color)], from_sq64);
        popBit(occupied_bitboard, from_sq64);

        setBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], to_sq64);
        setBit(color_bitboards[size_t(piece_color)], to_sq64);
        setBit(occupied_bitboard, to_sq64);
    }

    void clear_piece_sq64(int sq64) {
        DEBUG_ASSERT(sq64 >= 0 && sq64 < 64, "Cannot clear piece from invalid sq64");

        Piece piece = at_sq64(sq64);
        if (is_none(piece)) return; // Nothing to clear
        DEBUG_ASSERT(!is_offboard(piece), "Cannot clear offboard piece");

        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);

        material_score[size_t(piece_color)] -= value_of(piece);
        zobrist_key ^= Zobrist::Piece[int(piece_type) + (piece_color == Color::Black ? 6 : 0)][sq64];

        popBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], sq64);
        popBit(color_bitboards[size_t(piece_color)], sq64);
        popBit(occupied_bitboard, sq64);
    }

    void add_piece_sq64(int sq64, Piece piece) {
        DEBUG_ASSERT(sq64 >= 0 && sq64 < 64, "Cannot add piece to invalid sq64");
        DEBUG_ASSERT(!is_none(piece) && !is_offboard(piece), "Cannot add invalid piece");
        DEBUG_ASSERT(is_none(at_sq64(sq64)), "Cannot add piece to occupied square");

        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);

        if (piece_type != PieceType::King) {
            material_score[size_t(piece_color)] += value_of(piece);
        }
        zobrist_key ^= Zobrist::Piece[int(piece_type) + (piece_color == Color::Black ? 6 : 0)][sq64];

        setBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], sq64);
        setBit(color_bitboards[size_t(piece_color)], sq64);
        setBit(occupied_bitboard, sq64);
    }
    

    // (make_move_with_undo / undo_move dead code removed in Phase 4.8b � only
    // BitboardPosition::make_move_with_undo is used; the Position-class versions
    // had no external callers and depended on pList/pCount which are also gone.)

    
    // Material evaluation access functions
    int get_material_score(Color c) const {
        return material_score[size_t(c)];
    }
    
    int get_material_balance() const {
        return material_score[size_t(Color::White)] - material_score[size_t(Color::Black)];
    }
    
    int get_total_material() const {
        return material_score[size_t(Color::White)] + material_score[size_t(Color::Black)];
    }
    
    // Check if side has non-pawn material (for null move pruning)
    bool has_non_pawn_material(Color c) const {
        int ci = int(c);
        return (piece_bitboards[ci][int(PieceType::Queen)]
              | piece_bitboards[ci][int(PieceType::Rook)]
              | piece_bitboards[ci][int(PieceType::Bishop)]
              | piece_bitboards[ci][int(PieceType::Knight)]) != 0;
    }
    
    // Pawn bitboard access functions (derive from piece_bitboards)
    uint64_t get_pawn_bitboard(Color c) const {
        return piece_bitboards[size_t(c)][size_t(PieceType::Pawn)];
    }

    uint64_t get_all_pawns_bitboard() const {
        return piece_bitboards[size_t(Color::White)][size_t(PieceType::Pawn)] |
               piece_bitboards[size_t(Color::Black)][size_t(PieceType::Pawn)];
    }

    uint64_t get_white_pawns() const {
        return piece_bitboards[size_t(Color::White)][size_t(PieceType::Pawn)];
    }

    uint64_t get_black_pawns() const {
        return piece_bitboards[size_t(Color::Black)][size_t(PieceType::Pawn)];
    }
    
    // Full bitboard access functions for all piece types
    Bitboard get_piece_bitboard(Color color, PieceType piece_type) const {
        return piece_bitboards[size_t(color)][size_t(piece_type)];
    }
    
    Bitboard get_color_bitboard(Color color) const {
        return color_bitboards[size_t(color)];
    }
    
    Bitboard get_occupied_bitboard() const {
        return occupied_bitboard;
    }
    
    // Convenience accessors for specific piece types
    Bitboard get_pawns(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::Pawn)];
    }
    
    Bitboard get_knights(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::Knight)];
    }
    
    Bitboard get_bishops(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::Bishop)];
    }
    
    Bitboard get_rooks(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::Rook)];
    }
    
    Bitboard get_queens(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::Queen)];
    }
    
    Bitboard get_kings(Color color) const {
        return piece_bitboards[size_t(color)][size_t(PieceType::King)];
    }
    
    // Perft function for move generation validation - definition after class
    uint64_t perft(int depth);

    // Generate all moves for the current position - definition after class  
    void generate_all_moves(S_MOVELIST& list) const;
};

// Include S_MOVELIST definition after Position class declaration
#include "movegen.hpp"
