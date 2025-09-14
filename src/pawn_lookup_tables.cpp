/**
 * @file pawn_lookup_tables.cpp
 * @brief Implementation of pawn move lookup tables
 * 
 * Pre-computes all possible pawn move destinations for both White and Black pawns.
 * This eliminates runtime arithmetic in the critical path of pawn move generation,
 * targeting the 20.3% performance overhead identified in move generation profiling.
 * 
 * ## Initialization Process
 * 1. Allocate memory for lookup tables indexed by [Color][Square]
 * 2. For each square and color combination:
 *    - Calculate forward move destination
 *    - Calculate double move destination (from starting ranks only)
 *    - Calculate left and right capture destinations
 *    - Validate all moves with IS_PLAYABLE()
 *    - Store results or INVALID_SQUARE for impossible moves
 * 
 * ## Memory Usage
 * - 5 tables × 2 colors × 120 squares × 4 bytes = 4.8KB total
 * - Excellent cache locality due to sequential access patterns
 * 
 * @author MTDuke71
 * @version 1.0
 */

#include "pawn_lookup_tables.hpp"
#include "board120.hpp"
#include <iostream>

namespace PawnLookupTables {

// Lookup table storage - allocated dynamically during initialization
int (*PAWN_FORWARD_MOVES)[120] = nullptr;
int (*PAWN_DOUBLE_MOVES)[120] = nullptr;
int (*PAWN_CAPTURE_LEFT)[120] = nullptr;
int (*PAWN_CAPTURE_RIGHT)[120] = nullptr;
int (*PAWN_MOVE_MASK)[120] = nullptr;

void initialize_pawn_tables() {
    // Allocate memory for all lookup tables
    PAWN_FORWARD_MOVES = new int[2][120];
    PAWN_DOUBLE_MOVES = new int[2][120];
    PAWN_CAPTURE_LEFT = new int[2][120];
    PAWN_CAPTURE_RIGHT = new int[2][120];
    PAWN_MOVE_MASK = new int[2][120];
    
    // Initialize all tables to INVALID_SQUARE
    for (int color = 0; color < 2; ++color) {
        for (int square = 0; square < 120; ++square) {
            PAWN_FORWARD_MOVES[color][square] = INVALID_SQUARE;
            PAWN_DOUBLE_MOVES[color][square] = INVALID_SQUARE;
            PAWN_CAPTURE_LEFT[color][square] = INVALID_SQUARE;
            PAWN_CAPTURE_RIGHT[color][square] = INVALID_SQUARE;
            PAWN_MOVE_MASK[color][square] = 0;
        }
    }
    
    // Populate tables for all valid squares
    for (int square = 0; square < 120; ++square) {
        if (!IS_PLAYABLE(square)) continue;
        
        // Process both colors
        for (int color_int = 0; color_int < 2; ++color_int) {
            Color color = static_cast<Color>(color_int);
            
            // Determine pawn direction and starting rank
            int direction = (color == Color::White) ? NORTH : SOUTH;
            Rank starting_rank = (color == Color::White) ? Rank::R2 : Rank::R7;
            
            int move_mask = 0;
            
            // 1. Forward move (single step)
            int forward_dest = square + direction;
            if (IS_PLAYABLE(forward_dest)) {
                PAWN_FORWARD_MOVES[color_int][square] = forward_dest;
                move_mask |= FORWARD_MOVE_FLAG;
                
                // 2. Double move (only from starting rank)
                if (rank_of(square) == starting_rank) {
                    int double_dest = forward_dest + direction;
                    if (IS_PLAYABLE(double_dest)) {
                        PAWN_DOUBLE_MOVES[color_int][square] = double_dest;
                        move_mask |= DOUBLE_MOVE_FLAG;
                    }
                }
            }
            
            // 3. Left diagonal capture
            int left_capture_dest = square + direction + WEST;
            if (IS_PLAYABLE(left_capture_dest)) {
                // Additional check: ensure we don't wrap around the board
                File from_file = file_of(square);
                File to_file = file_of(left_capture_dest);
                
                // Valid if moving from file B-H to file A-G (left capture)
                if (from_file > File::A && to_file == static_cast<File>(int(from_file) - 1)) {
                    PAWN_CAPTURE_LEFT[color_int][square] = left_capture_dest;
                    move_mask |= LEFT_CAPTURE_FLAG;
                }
            }
            
            // 4. Right diagonal capture  
            int right_capture_dest = square + direction + EAST;
            if (IS_PLAYABLE(right_capture_dest)) {
                // Additional check: ensure we don't wrap around the board
                File from_file = file_of(square);
                File to_file = file_of(right_capture_dest);
                
                // Valid if moving from file A-G to file B-H (right capture)
                if (from_file < File::H && to_file == static_cast<File>(int(from_file) + 1)) {
                    PAWN_CAPTURE_RIGHT[color_int][square] = right_capture_dest;
                    move_mask |= RIGHT_CAPTURE_FLAG;
                }
            }
            
            // Store the move mask for this square and color
            PAWN_MOVE_MASK[color_int][square] = move_mask;
        }
    }
    
    std::cout << "Pawn lookup tables initialized successfully." << std::endl;
    
    // Debug: Print sample entries for verification
    std::cout << "Sample pawn moves for White pawn on e2 (square " << sq(File::E, Rank::R2) << "):" << std::endl;
    int e2 = sq(File::E, Rank::R2);
    std::cout << "  Forward: " << PAWN_FORWARD_MOVES[0][e2] << std::endl;
    std::cout << "  Double: " << PAWN_DOUBLE_MOVES[0][e2] << std::endl;
    std::cout << "  Left capture: " << PAWN_CAPTURE_LEFT[0][e2] << std::endl;
    std::cout << "  Right capture: " << PAWN_CAPTURE_RIGHT[0][e2] << std::endl;
    std::cout << "  Move mask: " << PAWN_MOVE_MASK[0][e2] << std::endl;
}

} // namespace PawnLookupTables