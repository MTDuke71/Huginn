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
 * - **Mailbox 120**: Primary board representation for fast move validation
 * - **Bitboards**: Secondary representation for attack detection and pattern recognition
 * - **Piece Lists**: Efficient iteration over pieces by type and color
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
#include "board120.hpp"
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
    
    // Derived state for incremental updates (performance optimization)
    std::array<int, 2> king_sq_backup;        // Previous king positions
    std::array<int, 2> material_score_backup; // Previous material scores
    
    // Constructor
    S_UNDO() : move(), castling_rights(0), ep_square(-1), halfmove_clock(0), zobrist_key(0), captured(Piece::None) {}
};

class Position {
public:
    Color side_to_move{Color::White};
    int ep_square{-1};               // mailbox-120 index or -1
    uint8_t castling_rights{0};      // bitmask: CASTLE_WK|CASTLE_WQ|CASTLE_BK|CASTLE_BQ
    uint16_t halfmove_clock{0};
    uint16_t fullmove_number{1};
    std::array<int, 2> king_sq{ -1, -1 }; // [White, Black] king locations (120)
    
    // Full bitboard representation for all piece types [Color][PieceType]
    std::array<std::array<Bitboard, int(PieceType::_Count)>, 2> piece_bitboards{};
    std::array<Bitboard, 2> color_bitboards{ 0, 0 }; // [White, Black] all pieces
    Bitboard occupied_bitboard{ 0 }; // All pieces (White | Black)

    // BACKLOG #26: piece-on-square cache for O(1) at_sq64 / at(sq120)
    // reads. Mirrors the piece bitboards; mutators (set, move_piece,
    // clear_piece, add_piece) update both in lock-step. Replaces the
    // 6-piece-type loop that at_sq64() used to do over the bitboards.
    std::array<Piece, 64> board64{};
    
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
    void save_derived_state(S_UNDO& undo);
    void rebuild_counts();
    void set_startpos();
    
    // VICE Tutorial Video #41: MakeMove function
    int MakeMove(const S_MOVE& move);
    
    // VICE Tutorial Video #42: TakeMove function
    void TakeMove();
    
    // VICE Part 83: Null move functions for null move pruning
    void MakeNullMove();
    void TakeNullMove();
    
    void restore_derived_state(const S_UNDO& undo) {
        king_sq = undo.king_sq_backup;
        material_score = undo.material_score_backup;
    }
    
    // Update derived state incrementally for a move (much faster than rebuild_counts).
    // Maintains material_score and king_sq[]. Pawn bitboard updates removed in 4.8a
    // (the legacy pawns_bb/all_pawns_bb fields are gone; piece_bitboards is the
    // single source of truth, updated separately by move_piece).
    void update_derived_state_for_move(const S_MOVE& m, Piece moving, Piece captured) {
        Color moving_color = color_of(moving);
        PieceType moving_type = type_of(moving);

        if (!is_none(captured) && type_of(captured) != PieceType::King) {
            Color captured_color = color_of(captured);
            if (captured_color != Color::None) {
                material_score[size_t(captured_color)] -= value_of(captured);
            }
        }

        if (m.is_promotion()) {
            material_score[size_t(moving_color)] -= value_of(make_piece(moving_color, PieceType::Pawn));
            material_score[size_t(moving_color)] += value_of(make_piece(moving_color, m.get_promoted()));
        } else if (moving_type == PieceType::King) {
            king_sq[size_t(moving_color)] = m.get_to();
        }
    }
    
    // Update Zobrist key incrementally for a move using XOR (much faster than recomputation)
    void update_zobrist_for_move(const S_MOVE& m, Piece moving, Piece captured, uint8_t old_castling_rights, int old_ep_square);
    
    // Compute and set the Zobrist key from current position
    void update_zobrist_key();

    // Access. at() returns Offboard for sentinel squares, otherwise
    // delegates to at_sq64 which reads the board64 piece cache.
    FORCE_INLINE Piece at(int s) const {
        if (s < 0 || s >= 120) return Piece::Offboard;
        int s64 = MAILBOX_MAPS.to64[s];
        if (s64 < 0) return Piece::Offboard;  // 120-sq sentinel
        return at_sq64(s64);
    }
    // Direct 64-square accessor for hot paths (movegen, eval, MakeMove)
    // that already hold the 64-square index. BACKLOG #26: this is now a
    // single array load instead of a bitboard scan with a 6-iteration
    // piece-type loop. Maintained by every Position mutator.
    FORCE_INLINE Piece at_sq64(int s64) const {
        assert(s64 >= 0 && s64 < 64);
        return board64[s64];
    }
    // Place / clear a piece at a 120-square index using bitboard storage.
    // If a piece was already at this square it is removed first; passing
    // Piece::None clears the square.
    inline void set(int s, Piece p) {
        if (!is_playable(s)) return;
        int s64 = MAILBOX_MAPS.to64[s];
        if (s64 < 0) return;
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
            board64[s64] = p;  // #26: piece-on-square cache
        } else {
            board64[s64] = Piece::None;  // #26
        }
    }

    // Atomic piece movement - follows VICE MovePiece pattern
    // Moves a piece from one square to another, updating all necessary data structures
    void move_piece(int from_square, int to_square) {
        DEBUG_ASSERT(is_playable(from_square), "Invalid source square for piece move");
        DEBUG_ASSERT(is_playable(to_square), "Invalid destination square for piece move");
        
        Piece piece = at(from_square);
        DEBUG_ASSERT(!is_none(piece), "Cannot move piece from empty square");
        DEBUG_ASSERT(is_none(at(to_square)), "Cannot move piece to occupied square");
        
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // 1. Hash piece out of from square and into to square
        zobrist_key ^= Zobrist::Piece[int(type_of(piece)) + (color_of(piece) == Color::Black ? 6 : 0)][from_square];
        zobrist_key ^= Zobrist::Piece[int(type_of(piece)) + (color_of(piece) == Color::Black ? 6 : 0)][to_square];

        // 2. Update bitboards
        int from_sq64 = MAILBOX_MAPS.to64[from_square];
        int to_sq64 = MAILBOX_MAPS.to64[to_square];
        if (from_sq64 >= 0 && to_sq64 >= 0) {
            // Update new full bitboard system
            popBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], from_sq64);
            popBit(color_bitboards[size_t(piece_color)], from_sq64);
            popBit(occupied_bitboard, from_sq64);

            setBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], to_sq64);
            setBit(color_bitboards[size_t(piece_color)], to_sq64);
            setBit(occupied_bitboard, to_sq64);

            // #26: keep board64 in sync
            board64[from_sq64] = Piece::None;
            board64[to_sq64]   = piece;
        }
    }
    
    // Atomic piece removal - consolidates all operations for better performance
    // Follows the VICE ClearPiece pattern but maintains Huginn's C++ style
    void clear_piece(int square) {
        DEBUG_ASSERT(is_playable(square), "Cannot clear piece from invalid square");
        
        Piece piece = at(square);
        if (is_none(piece)) return; // Nothing to clear
        
        DEBUG_ASSERT(!is_offboard(piece), "Cannot clear offboard piece");
        
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // 1. Update zobrist hash (XOR out the piece)
        zobrist_key ^= Zobrist::Piece[int(type_of(piece)) + (color_of(piece) == Color::Black ? 6 : 0)][square];

        // 2. Update material score (kings can never be captured in chess)
        material_score[size_t(piece_color)] -= value_of(piece);

        // 3. Update bitboards
        int sq64 = MAILBOX_MAPS.to64[square];
        if (sq64 >= 0) {
            popBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], sq64);
            popBit(color_bitboards[size_t(piece_color)], sq64);
            popBit(occupied_bitboard, sq64);
            board64[sq64] = Piece::None;  // #26
        }
    }

    // Atomic piece addition - complements clear_piece for better performance
    // Follows the VICE AddPiece pattern but maintains Huginn's C++ style
    void add_piece(int square, Piece piece) {
        DEBUG_ASSERT(is_playable(square), "Cannot add piece to invalid square");
        DEBUG_ASSERT(!is_none(piece) && !is_offboard(piece), "Cannot add invalid piece");
        DEBUG_ASSERT(is_none(at(square)), "Cannot add piece to occupied square");
        
        Color piece_color = color_of(piece);
        PieceType piece_type = type_of(piece);
        
        // 1. Update zobrist hash (XOR in the piece)
        zobrist_key ^= Zobrist::Piece[int(type_of(piece)) + (color_of(piece) == Color::Black ? 6 : 0)][square];

        // 2. Update material score
        if (piece_type != PieceType::King) {
            material_score[size_t(piece_color)] += value_of(piece);
        }

        // 3. Update bitboards
        int sq64 = MAILBOX_MAPS.to64[square];
        if (sq64 >= 0) {
            setBit(piece_bitboards[size_t(piece_color)][size_t(piece_type)], sq64);
            setBit(color_bitboards[size_t(piece_color)], sq64);
            setBit(occupied_bitboard, sq64);
            board64[sq64] = piece;  // #26
        }
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
