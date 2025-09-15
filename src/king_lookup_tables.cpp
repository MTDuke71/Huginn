/**
 * @file king_lookup_tables.cpp
 * @brief Implementation of king move lookup tables
 * @author Huginn Chess Engine Development Team
 * @date September 2025
 * @version 1.0
 * 
 * This file implements the initialization and debugging functions for king move
 * lookup tables. The tables are populated once at engine startup and provide
 * optimized king move generation throughout the game.
 */

#include "king_lookup_tables.hpp"
#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "board120.hpp"
#include "msvc_intrinsics.hpp"
#include <iostream>
#include <iomanip>
#include <cstring>

namespace KingLookupTables {

    /// @brief Storage for pre-computed king move arrays
    int KING_MOVES[64][8];
    
    /// @brief Storage for move counts per square
    int KING_MOVE_COUNT[64];
    
    /// @brief Storage for bitboard attack patterns
    uint64_t KING_ATTACKS[64];
    
    /**
     * @brief Internal function to compute king moves for a 64-square index
     * @param square Square index (0-63) in 64-square representation
     * @param moves Output array for destination squares
     * @return Number of valid moves found
     * 
     * @details Computes all legal king moves from the given square using
     *          the 8 directional offsets. Handles board boundaries correctly.
     */
    static int compute_king_moves(int square, int moves[8]) {
        // King move offsets in 64-square board
        // Note: These are file/rank differences, not 120-square offsets
        static const int king_offsets[8][2] = {
            {-1, -1}, {-1,  0}, {-1,  1},  // Up-left, Up, Up-right
            { 0, -1},           { 0,  1},  // Left, Right
            { 1, -1}, { 1,  0}, { 1,  1}   // Down-left, Down, Down-right
        };
        
        int file = square % 8;  // Files 0-7 (a-h)
        int rank = square / 8;  // Ranks 0-7 (1-8)
        int count = 0;
        
        for (int i = 0; i < 8; ++i) {
            int new_rank = rank + king_offsets[i][0];
            int new_file = file + king_offsets[i][1];
            
            // Check boundaries
            if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
                moves[count++] = new_rank * 8 + new_file;
            }
        }
        
        return count;
    }
    
    /**
     * @brief Internal function to compute king attack bitboard for a square
     * @param square Square index (0-63) in 64-square representation
     * @return Bitboard with bits set for all attacked squares
     * 
     * @details Creates a bitboard representation of king attacks for bitboard-based
     *          move generation algorithms.
     */
    static uint64_t compute_king_bitboard(int square) {
        uint64_t attacks = 0ULL;
        
        int file = square % 8;
        int rank = square / 8;
        
        // Same 8 directions as array version
        static const int king_offsets[8][2] = {
            {-1, -1}, {-1,  0}, {-1,  1},  
            { 0, -1},           { 0,  1},  
            { 1, -1}, { 1,  0}, { 1,  1}   
        };
        
        for (int i = 0; i < 8; ++i) {
            int new_rank = rank + king_offsets[i][0];
            int new_file = file + king_offsets[i][1];
            
            if (new_rank >= 0 && new_rank < 8 && new_file >= 0 && new_file < 8) {
                int target_square = new_rank * 8 + new_file;
                attacks |= (1ULL << target_square);
            }
        }
        
        return attacks;
    }
    
    void initialize_king_tables() {
        // Clear all tables first
        std::memset(KING_MOVES, 0, sizeof(KING_MOVES));
        std::memset(KING_MOVE_COUNT, 0, sizeof(KING_MOVE_COUNT));
        std::memset(KING_ATTACKS, 0, sizeof(KING_ATTACKS));
        
        // Populate tables for all 64 squares
        for (int square = 0; square < 64; ++square) {
            // Generate array-based moves
            KING_MOVE_COUNT[square] = compute_king_moves(square, KING_MOVES[square]);
            
            // Generate bitboard representation
            KING_ATTACKS[square] = compute_king_bitboard(square);
        }
        
        // Verify consistency between array and bitboard representations
        #ifndef NDEBUG
        for (int square = 0; square < 64; ++square) {
            // Count bits in bitboard should equal array move count
            int bitboard_count = builtin_popcountll(KING_ATTACKS[square]);
            if (bitboard_count != KING_MOVE_COUNT[square]) {
                std::cerr << "Warning: King table inconsistency at square " << square 
                         << " (array: " << KING_MOVE_COUNT[square] 
                         << ", bitboard: " << bitboard_count << ")" << std::endl;
            }
            
            // Verify each array move corresponds to a bit in the bitboard
            for (int i = 0; i < KING_MOVE_COUNT[square]; ++i) {
                int move_square = KING_MOVES[square][i];
                if ((KING_ATTACKS[square] & (1ULL << move_square)) == 0) {
                    std::cerr << "Warning: Array move " << move_square 
                             << " not found in bitboard for square " << square << std::endl;
                }
            }
        }
        #endif
    }
    
    void print_king_tables() {
        std::cout << "\n=== King Lookup Tables Debug Output ===\n";
        std::cout << "Format: [square] file(square_name) -> moves (count) | bitboard_hex\n\n";
        
        // Memory usage statistics
        size_t array_memory = sizeof(KING_MOVES) + sizeof(KING_MOVE_COUNT);
        size_t bitboard_memory = sizeof(KING_ATTACKS);
        size_t total_memory = array_memory + bitboard_memory;
        
        std::cout << "Memory Usage:\n";
        std::cout << "  Array tables:  " << array_memory << " bytes\n";
        std::cout << "  Bitboard table: " << bitboard_memory << " bytes\n";
        std::cout << "  Total:         " << total_memory << " bytes\n";
        std::cout << "  Cache lines:   " << (total_memory + 63) / 64 << " (64-byte lines)\n\n";
        
        // Convert square index to algebraic notation
        auto square_to_algebraic = [](int square) -> std::string {
            int file = square % 8;
            int rank = square / 8;
            return std::string(1, 'a' + file) + std::to_string(rank + 1);
        };
        
        // Print table for each square
        for (int square = 0; square < 64; ++square) {
            std::cout << "[" << std::setw(2) << square << "] " 
                     << square_to_algebraic(square) << " -> ";
            
            // Print array moves
            for (int i = 0; i < KING_MOVE_COUNT[square]; ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << square_to_algebraic(KING_MOVES[square][i]);
            }
            
            std::cout << " (" << KING_MOVE_COUNT[square] << ")";
            
            // Print bitboard in hex
            std::cout << " | 0x" << std::hex << std::setw(16) << std::setfill('0') 
                     << KING_ATTACKS[square] << std::dec << std::setfill(' ');
            
            // Verify consistency
            int bitboard_count = builtin_popcountll(KING_ATTACKS[square]);
            if (bitboard_count != KING_MOVE_COUNT[square]) {
                std::cout << " [MISMATCH!]";
            }
            
            std::cout << "\n";
            
            // Add separator after each rank for readability
            if ((square + 1) % 8 == 0) {
                std::cout << "\n";
            }
        }
        
        // Summary statistics
        int total_moves = 0;
        int corner_squares = 0, edge_squares = 0, center_squares = 0;
        
        for (int square = 0; square < 64; ++square) {
            total_moves += KING_MOVE_COUNT[square];
            
            int file = square % 8;
            int rank = square / 8;
            
            if ((file == 0 || file == 7) && (rank == 0 || rank == 7)) {
                corner_squares++;
            } else if (file == 0 || file == 7 || rank == 0 || rank == 7) {
                edge_squares++;
            } else {
                center_squares++;
            }
        }
        
        std::cout << "Summary Statistics:\n";
        std::cout << "  Total moves: " << total_moves << " (average: " 
                 << (double)total_moves / 64 << " per square)\n";
        std::cout << "  Corner squares: " << corner_squares << " (3 moves each)\n";
        std::cout << "  Edge squares: " << edge_squares << " (5 moves each)\n";
        std::cout << "  Center squares: " << center_squares << " (8 moves each)\n";
        std::cout << "  Expected total: " << (corner_squares * 3 + edge_squares * 5 + center_squares * 8) << "\n";
        
        std::cout << "\n=== End King Lookup Tables ===\n\n";
    }
    
    void generate_castling_moves_optimized(const Position& pos, S_MOVELIST& list, Color us) {
        // Calculate castle squares using the same logic as CastlingSquares
        constexpr int WHITE_KING_START = sq(File::E, Rank::R1);
        constexpr int WHITE_KINGSIDE_KING_TO = sq(File::G, Rank::R1);
        constexpr int WHITE_QUEENSIDE_KING_TO = sq(File::C, Rank::R1);
        constexpr int BLACK_KING_START = sq(File::E, Rank::R8);
        constexpr int BLACK_KINGSIDE_KING_TO = sq(File::G, Rank::R8);
        constexpr int BLACK_QUEENSIDE_KING_TO = sq(File::C, Rank::R8);
        
        if (us == Color::White) {
            // White kingside castling (e1-g1)
            if (pos.castling_rights & CASTLE_WK) {
                const int f1 = sq(File::F, Rank::R1);
                const int g1 = sq(File::G, Rank::R1);
                
                if (pos.board[f1] == Piece::None && pos.board[g1] == Piece::None) {
                    const int e1 = WHITE_KING_START;
                    if (!Huginn::SqAttacked(e1, pos, Color::Black) && 
                        !Huginn::SqAttacked(f1, pos, Color::Black) && 
                        !Huginn::SqAttacked(g1, pos, Color::Black)) {
                        list.add_quiet_move(make_castle(e1, WHITE_KINGSIDE_KING_TO));
                    }
                }
            }
            
            // White queenside castling (e1-c1)  
            if (pos.castling_rights & CASTLE_WQ) {
                const int d1 = sq(File::D, Rank::R1);
                const int c1 = sq(File::C, Rank::R1);
                const int b1 = sq(File::B, Rank::R1);
                
                if (pos.board[d1] == Piece::None && pos.board[c1] == Piece::None && pos.board[b1] == Piece::None) {
                    const int e1 = WHITE_KING_START;
                    if (!Huginn::SqAttacked(e1, pos, Color::Black) && 
                        !Huginn::SqAttacked(d1, pos, Color::Black) && 
                        !Huginn::SqAttacked(c1, pos, Color::Black)) {
                        list.add_quiet_move(make_castle(e1, WHITE_QUEENSIDE_KING_TO));
                    }
                }
            }
        } else {
            // Black kingside castling (e8-g8)
            if (pos.castling_rights & CASTLE_BK) {
                const int f8 = sq(File::F, Rank::R8);
                const int g8 = sq(File::G, Rank::R8);
                
                if (pos.board[f8] == Piece::None && pos.board[g8] == Piece::None) {
                    const int e8 = BLACK_KING_START;
                    if (!Huginn::SqAttacked(e8, pos, Color::White) && 
                        !Huginn::SqAttacked(f8, pos, Color::White) && 
                        !Huginn::SqAttacked(g8, pos, Color::White)) {
                        list.add_quiet_move(make_castle(e8, BLACK_KINGSIDE_KING_TO));
                    }
                }
            }
            
            // Black queenside castling (e8-c8)
            if (pos.castling_rights & CASTLE_BQ) {
                const int d8 = sq(File::D, Rank::R8);
                const int c8 = sq(File::C, Rank::R8);
                const int b8 = sq(File::B, Rank::R8);
                
                if (pos.board[d8] == Piece::None && pos.board[c8] == Piece::None && pos.board[b8] == Piece::None) {
                    const int e8 = BLACK_KING_START;
                    if (!Huginn::SqAttacked(e8, pos, Color::White) && 
                        !Huginn::SqAttacked(d8, pos, Color::White) && 
                        !Huginn::SqAttacked(c8, pos, Color::White)) {
                        list.add_quiet_move(make_castle(e8, BLACK_QUEENSIDE_KING_TO));
                    }
                }
            }
        }
    }

} // namespace KingLookupTables