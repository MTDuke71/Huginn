/**
 * @file debug_board_state.cpp
 * @brief Debug what pieces UltraEngine thinks are on the board
 */
#include <iostream>
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

// Convert 64-bit square index to algebraic notation
std::string square_to_algebraic(int square) {
    int file = square % 8;  // 0-7 for a-h
    int rank = square / 8;  // 0-7 for 1-8
    char file_char = 'a' + file;
    char rank_char = '1' + rank;
    return std::string(1, file_char) + std::string(1, rank_char);
}

char piece_to_char(int piece_info) {
    if (piece_info == -1) return '.';  // Empty square
    
    int color = (piece_info >> 3) & 1;  // Extract color
    int piece = piece_info & 7;         // Extract piece type
    
    char pieces[] = {'P', 'N', 'B', 'R', 'Q', 'K'};  // White pieces
    if (piece < 0 || piece >= 6) return '?';
    
    char piece_char = pieces[piece];
    return color == 0 ? piece_char : (piece_char + 32);  // Lowercase for black
}

int main() {
    UltraAttacks::initialize();
    
    // Kiwipete position FEN
    std::string fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    
    UltraPosition pos;
    if (!pos.set_fen(fen)) {
        std::cout << "Failed to parse FEN: " << fen << std::endl;
        return 1;
    }
    
    std::cout << "=== BOARD STATE AFTER FEN PARSING ===" << std::endl;
    std::cout << "FEN: " << fen << std::endl;
    std::cout << std::endl;
    
    // Print the board from UltraEngine's perspective
    std::cout << "Board (rank 8 to 1, file a to h):" << std::endl;
    std::cout << "  a b c d e f g h" << std::endl;
    
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            int piece_info = pos.get_piece_at(square);
            std::cout << piece_to_char(piece_info) << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << std::endl;
    
    // Check specific squares that are generating moves incorrectly
    std::cout << "=== CHECKING SPECIFIC SQUARES ===" << std::endl;
    std::vector<std::string> problem_squares = {"a4", "a7", "b4", "c2", "d4", "e7", "f4", "g4", "h7"};
    
    for (const std::string& sq_name : problem_squares) {
        // Convert algebraic to square index
        int file = sq_name[0] - 'a';
        int rank = sq_name[1] - '1';
        int square = rank * 8 + file;
        
        int piece_info = pos.get_piece_at(square);
        std::cout << "Square " << sq_name << " (index " << square << "): " 
                  << piece_to_char(piece_info);
        if (piece_info == -1) {
            std::cout << " (EMPTY)";
        } else {
            int color = (piece_info >> 3) & 1;
            int piece = piece_info & 7;
            std::cout << " (color=" << color << ", piece=" << piece << ")";
        }
        std::cout << std::endl;
    }
    
    return 0;
}