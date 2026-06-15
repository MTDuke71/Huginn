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

/**
 * @brief Minimal irreversible-state snapshot for a single make/unmake.
 *
 * Captures the position fields that a move mutates but cannot be recomputed
 * from the board alone, so `TakeMove` can restore them exactly. Superseded in
 * the main search path by the richer ::S_UNDO (which also stores the move and
 * Zobrist key); kept for lightweight call sites.
 */
struct State {
    int ep_square;            ///< En passant target square (sq64) before the move, or -1.
    uint8_t castling_rights;  ///< Castling-rights bitmask before the move.
    uint16_t halfmove_clock;  ///< Fifty-move halfmove counter before the move.
    Piece captured;           ///< Piece captured by the move (::Piece::None if none).
};

/**
 * @brief Full undo record pushed onto Position::move_history per `MakeMove`.
 *
 * Holds everything `TakeMove` needs to reverse a move in O(1): the move itself
 * plus the irreversible state it overwrote. King squares and material score are
 * deliberately *not* stored — `TakeMove` restores `king_sq` directly and material
 * is maintained incrementally by the atomic piece ops (see Priority 7 note).
 */
struct S_UNDO {
    S_MOVE move;              ///< The move that was made (full packed encoding).
    uint8_t castling_rights;  ///< Castling permissions before the move (castlePerm).
    int ep_square;            ///< En passant square before the move (enPas), or -1.
    uint16_t halfmove_clock;  ///< Fifty-move counter before the move (fiftyMove).
    uint64_t zobrist_key;     ///< Position hash before the move (posKey).
    Piece captured;           ///< Piece captured by the move (::Piece::None if none).

    // (king_sq / material_score backups removed: TakeMove restores king_sq
    // directly and material_score is maintained incrementally by the atomic
    // piece ops, so the backups were write-only — see Priority 7.)

    // Constructor
    S_UNDO() : move(), castling_rights(0), ep_square(-1), halfmove_clock(0), zobrist_key(0), captured(Piece::None) {}
};

/**
 * @brief Complete chess position — the engine's central data structure.
 *
 * Per-piece bitboards are the single source of truth for placement; the color
 * and occupancy bitboards are derived caches kept in lock-step. Piece-on-square
 * lookups go through at_sq64() (there is no mailbox board). Make/unmake is O(1)
 * and incremental: each `MakeMove` pushes an ::S_UNDO and updates the Zobrist
 * key and material score atomically, and `TakeMove` reverses it exactly.
 *
 * Squares are indexed 0..63 (a1=0, h8=63). All public member fields are mutated
 * in place by make/unmake — copy a Position only when you need an independent
 * snapshot.
 */
class Position {
public:
    Color side_to_move{Color::White}; ///< Side to move.
    int ep_square{-1};               ///< En passant target square (sq64 0..63), or -1 if none.
    uint8_t castling_rights{0};      ///< Castling bitmask: CASTLE_WK|CASTLE_WQ|CASTLE_BK|CASTLE_BQ.
    uint16_t halfmove_clock{0};      ///< Halfmoves since last capture/pawn move (fifty-move rule).
    uint16_t fullmove_number{1};     ///< Full-move counter (increments after Black moves).
    std::array<int, 2> king_sq{ -1, -1 }; ///< King squares (sq64) indexed [White, Black]; -1 if absent.

    /// Per-piece bitboards, indexed `[Color][PieceType]` — the sole board state.
    std::array<std::array<Bitboard, int(PieceType::_Count)>, 2> piece_bitboards{};
    std::array<Bitboard, 2> color_bitboards{ 0, 0 }; ///< All pieces of each side, indexed [White, Black] (derived).
    Bitboard occupied_bitboard{ 0 }; ///< Union of both colors' pieces (derived).

    uint64_t zobrist_key{0};         ///< Incremental Zobrist hash (repetition detection + TT key).

    std::array<int, 2> material_score{ 0, 0 }; ///< Per-side material total (cp), indexed [White, Black].

    std::vector<S_UNDO> move_history; ///< Undo stack; one ::S_UNDO per made move.
    int ply{0};                      ///< Current search/game ply (depth from the root).

    /// Constructs the empty position and reserves undo-stack capacity.
    Position() : move_history() {
        move_history.reserve(64);  // Reserve reasonable initial capacity
    }

    /// Clears the board to an empty position (all bitboards/state zeroed).
    void reset();

    /**
     * @brief Sets the position from a FEN string.
     * @param fen Full FEN record (placement, side, castling, ep, clocks).
     * @return true on success; false if the FEN is malformed (position left undefined).
     */
    bool set_from_fen(const std::string& fen);

    /// @return The current position serialized as a FEN string.
    std::string to_fen() const;

    /// Recomputes derived caches (color/occupancy bitboards, material, king_sq) from the per-piece bitboards.
    void rebuild_counts();

    /// Sets the standard chess starting position.
    void set_startpos();

    /**
     * @brief Makes a move, updating all state and pushing an undo record. (VICE #41)
     * @param move The (pseudo-legal) move to play.
     * @return 1 if the move is legal and was made; 0 if it left the mover's king in
     *         check, in which case the position is restored and nothing is pushed.
     */
    int MakeMove(const S_MOVE& move);

    /// Reverses the most recent MakeMove, popping the undo stack. (VICE #42)
    void TakeMove();

    /// Makes a null move (pass) for null-move pruning: flips side, clears ep. (VICE #83)
    void MakeNullMove();
    /// Reverses MakeNullMove, restoring side-to-move and en passant.
    void TakeNullMove();

    /**
     * @brief Updates the Zobrist key incrementally for a move via XOR (no full recompute).
     * @param m The move being applied.
     * @param moving The piece that moves.
     * @param captured The captured piece, or ::Piece::None.
     * @param old_castling_rights Castling rights before the move (to XOR out).
     * @param old_ep_square En passant square before the move (to XOR out).
     */
    void update_zobrist_for_move(const S_MOVE& m, Piece moving, Piece captured, uint8_t old_castling_rights, int old_ep_square);

    /// Recomputes the full Zobrist key from the current position (non-incremental).
    void update_zobrist_key();

    /**
     * @brief Returns the piece on a 64-square index, derived from the bitboards.
     * @param s64 Square index in [0, 64) (caller-guaranteed).
     * @return The piece occupying the square, or ::Piece::None if empty.
     *
     * History: BACKLOG #26 (e61f6e5) added a board64[64] piece-on-square
     * cache to make this an array load; bench gained +12% NPS but pooled
     * 400g vs t5 came in at -13 Elo (Intel +12 / AMD -38). The invariant
     * test (b8cd310) confirmed the cache was NOT desyncing — the +64
     * bytes of cache footprint cost as much as the loop saved on this
     * codebase. Reverted; bitboard scan kept.
     */
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
    /**
     * @brief Places (or clears) a piece at a square, keeping all bitboards in sync.
     * @param s64 Square index in [0, 64) (caller-guaranteed).
     * @param p Piece to place; any existing occupant is removed first. Pass
     *          ::Piece::None to clear the square.
     * @note Setup/FEN helper — does NOT update Zobrist or material. Use the
     *       atomic `*_sq64` ops on the make/unmake hot path instead.
     */
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

    /**
     * @brief Moves a piece between empty-to-occupied squares, updating Zobrist.
     * @param from_sq64 Source square (must hold a piece).
     * @param to_sq64 Destination square (must be empty).
     * @note Material is unchanged (no capture). Captures are modeled as a
     *       separate clear_piece_sq64() then move_piece_sq64().
     */
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

    /**
     * @brief Removes the piece on a square, updating material and Zobrist.
     * @param sq64 Square to clear; a no-op if already empty.
     */
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

    /**
     * @brief Adds a piece to an empty square, updating material and Zobrist.
     * @param sq64 Destination square (must be empty).
     * @param piece Piece to add (must not be ::Piece::None / Offboard).
     * @note King material is intentionally not added (kings carry no material value).
     */
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

    
    /// @return The side's incrementally-tracked material total (cp).
    int get_material_score(Color c) const {
        return material_score[size_t(c)];
    }

    /// @return Material balance from White's perspective (White − Black, cp).
    int get_material_balance() const {
        return material_score[size_t(Color::White)] - material_score[size_t(Color::Black)];
    }

    /// @return Total material on the board (White + Black, cp) — used for phase detection.
    int get_total_material() const {
        return material_score[size_t(Color::White)] + material_score[size_t(Color::Black)];
    }

    /**
     * @brief Tests whether a side has any non-pawn, non-king material.
     * @param c Side to test.
     * @return true if `c` has at least one queen/rook/bishop/knight.
     * @note Used to disable null-move pruning in zugzwang-prone pawn endings.
     */
    bool has_non_pawn_material(Color c) const {
        int ci = int(c);
        return (piece_bitboards[ci][int(PieceType::Queen)]
              | piece_bitboards[ci][int(PieceType::Rook)]
              | piece_bitboards[ci][int(PieceType::Bishop)]
              | piece_bitboards[ci][int(PieceType::Knight)]) != 0;
    }
    
    /// @return Bitboard of all pawns of both colors.
    uint64_t get_all_pawns_bitboard() const {
        return piece_bitboards[size_t(Color::White)][size_t(PieceType::Pawn)] |
               piece_bitboards[size_t(Color::Black)][size_t(PieceType::Pawn)];
    }

    /// @return Bitboard of the White pawns.
    uint64_t get_white_pawns() const {
        return piece_bitboards[size_t(Color::White)][size_t(PieceType::Pawn)];
    }

    /// @return Bitboard of the Black pawns.
    uint64_t get_black_pawns() const {
        return piece_bitboards[size_t(Color::Black)][size_t(PieceType::Pawn)];
    }

    /**
     * @brief Counts leaf nodes of the legal move tree to a fixed depth (movegen validation).
     * @param depth Plies to expand.
     * @return Number of legal positions at exactly `depth`.
     */
    uint64_t perft(int depth);

    /**
     * @brief Generates all pseudo-legal moves for the side to move.
     * @param[out] list Move list to fill (cleared/overwritten).
     */
    void generate_all_moves(S_MOVELIST& list) const;
};

// Include S_MOVELIST definition after Position class declaration
#include "movegen.hpp"
