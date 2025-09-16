/**
 * @file bitboard_position.hpp
 * @brief Pure 64-square bitboard position representation
 * 
 * This file implements a parallel chess position architecture using native bitboards
 * without any 120-square dependencies. Designed to eliminate conversion overhead and
 * maximize bitboard operation efficiency.
 * 
 * ## Architecture Goals
 * - **Zero conversion overhead**: All operations in native 64-square space
 * - **Bitboard-native**: Piece representation using uint64_t bitboards
 * - **Parallel coexistence**: Can work alongside 120-square Position class
 * - **Performance focus**: Optimized for move generation and evaluation
 * 
 * ## Key Differences from 120-square Position
 * - All squares represented as 0-63 indices (standard chess board)
 * - No board[] array - pieces tracked via bitboards only
 * - No piece lists - iteration via bitboard operations
 * - No MAILBOX_MAPS dependencies
 * - Direct bitboard move generation without conversions
 * 
 * @author MTDuke71
 * @version 1.0
 * @see position.hpp for 120-square architecture
 */
#pragma once

#include <cstdint>
#include <array>
#include <string>
#include "chess_types.hpp"

// ============================================================================
// BITBOARD POSITION ARCHITECTURE
// ============================================================================

/**
 * @brief Pure bitboard chess position representation
 * 
 * Native 64-square position class that avoids all 120-square conversions.
 * Uses bitboards as the primary representation for maximum performance.
 */
class BitboardPosition {
public:
    // ---- Core bitboard representation ----
    std::array<std::array<uint64_t, 7>, 2> piece_bitboards;  // [color][piece_type]
    std::array<uint64_t, 2> color_bitboards;                 // [color] - all pieces of that color
    uint64_t occupied_bitboard;                               // All occupied squares
    
    // ---- Game state ----
    Color side_to_move;
    int ep_square_64;                                         // En passant in 64-square format (-1 if none)
    uint8_t castling_rights;                                  // Castling permissions bitmask
    uint16_t halfmove_clock;
    uint16_t fullmove_number;
    
    // ---- Fast lookups for special pieces ----
    std::array<int, 2> king_square_64;                       // King positions in 64-square format
    
    // ---- Material tracking ----
    std::array<int, 2> material_score;                       // [white, black] material balance
    
    // ---- Zobrist and history ----
    uint64_t zobrist_key;
    int ply;
    
    // ---- Construction and initialization ----
    BitboardPosition();
    void reset();
    bool set_from_fen(const std::string& fen);
    std::string to_fen() const;
    
    // ---- Square access (64-square native) ----
    Piece piece_at(int square_64) const;
    Color color_at(int square_64) const;
    PieceType piece_type_at(int square_64) const;
    
    // ---- Piece manipulation ----
    void place_piece(int square_64, Color color, PieceType piece_type);
    void remove_piece(int square_64);
    void move_piece(int from_64, int to_64);
    
    // ---- Bitboard accessors ----
    uint64_t get_pieces(Color color, PieceType piece_type) const;
    uint64_t get_all_pieces(Color color) const;
    uint64_t get_occupied() const { return occupied_bitboard; }
    uint64_t get_empty() const { return ~occupied_bitboard; }
    
    // ---- Utility functions ----
    bool is_square_occupied(int square_64) const;
    bool is_enemy_piece(int square_64, Color our_color) const;
    bool is_our_piece(int square_64, Color our_color) const;
    
    // ---- Conversion utilities (for interfacing with 120-square code) ----
    void sync_from_120_position(const class Position& pos_120);
    void sync_to_120_position(class Position& pos_120) const;
    
private:
    // ---- Internal bitboard management ----
    void update_color_bitboards();
    void update_occupied_bitboard();
    void update_derived_bitboards();
    
    // ---- FEN parsing helpers ----
    bool parse_fen_board(const std::string& board_part);
    bool parse_fen_metadata(const std::string& metadata_part);
};

// ============================================================================
// BITBOARD CONSTANTS AND UTILITIES
// ============================================================================

// Standard 64-square board layout (0-63)
// Rank 1 = 0-7, Rank 2 = 8-15, ..., Rank 8 = 56-63
// File A = 0,8,16,24,32,40,48,56, File H = 7,15,23,31,39,47,55,63

constexpr int SQUARE_A1 = 0;
constexpr int SQUARE_H1 = 7;
constexpr int SQUARE_A8 = 56;
constexpr int SQUARE_H8 = 63;

// Rank and file extraction from 64-square index
constexpr int rank_of_64(int square_64) { return square_64 / 8; }
constexpr int file_of_64(int square_64) { return square_64 % 8; }
constexpr int square_64(int file, int rank) { return rank * 8 + file; }

// Bitboard rank and file masks
constexpr uint64_t RANK_1_BB = 0x00000000000000FFULL;
constexpr uint64_t RANK_2_BB = 0x000000000000FF00ULL;
constexpr uint64_t RANK_3_BB = 0x0000000000FF0000ULL;
constexpr uint64_t RANK_4_BB = 0x00000000FF000000ULL;
constexpr uint64_t RANK_5_BB = 0x000000FF00000000ULL;
constexpr uint64_t RANK_6_BB = 0x0000FF0000000000ULL;
constexpr uint64_t RANK_7_BB = 0x00FF000000000000ULL;
constexpr uint64_t RANK_8_BB = 0xFF00000000000000ULL;

constexpr uint64_t FILE_A_BB = 0x0101010101010101ULL;
constexpr uint64_t FILE_B_BB = 0x0202020202020202ULL;
constexpr uint64_t FILE_C_BB = 0x0404040404040404ULL;
constexpr uint64_t FILE_D_BB = 0x0808080808080808ULL;
constexpr uint64_t FILE_E_BB = 0x1010101010101010ULL;
constexpr uint64_t FILE_F_BB = 0x2020202020202020ULL;
constexpr uint64_t FILE_G_BB = 0x4040404040404040ULL;
constexpr uint64_t FILE_H_BB = 0x8080808080808080ULL;

// ============================================================================
// BITBOARD MOVE GENERATION INTERFACE
// ============================================================================

// Forward declarations for move generation
struct BitboardMoveList;

namespace BitboardMoveGen {
    void generate_all_moves(const BitboardPosition& pos, BitboardMoveList& moves);
    void generate_pawn_moves(const BitboardPosition& pos, BitboardMoveList& moves);
    void generate_knight_moves(const BitboardPosition& pos, BitboardMoveList& moves);
    void generate_bishop_moves(const BitboardPosition& pos, BitboardMoveList& moves);
    void generate_rook_moves(const BitboardPosition& pos, BitboardMoveList& moves);
    void generate_queen_moves(const BitboardPosition& pos, BitboardMoveList& moves);
    void generate_king_moves(const BitboardPosition& pos, BitboardMoveList& moves);
}