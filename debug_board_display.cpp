/**
 * @file debug_board_display.cpp
 * @brief Show what pieces UltraEngine thinks are on the board
 */
#include <iostream>
#include <iomanip>
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

char piece_to_char(int piece_info) {
    if (piece_info == -1) return '.';  // Empty square
    
    int color = (piece_info >> 3) & 1;  // Extract color (0=white, 1=black)
    int piece = piece_info & 7;         // Extract piece type
    
    char pieces[] = {'P', 'N', 'B', 'R', 'Q', 'K'};  // White pieces
    if (piece < 0 || piece >= 6) return '?';
    
    char piece_char = pieces[piece];
    return color == 0 ? piece_char : (piece_char + 32);  // Lowercase for black
}

int main() {
    UltraAttacks::initialize();
    
    // Kiwipete position (CORRECT)
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    UltraPosition pos;
    if (!pos.set_fen(fen)) {
        std::cout << "Failed to parse FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "=== KIWIPETE POSITION BOARD STATE ===" << std::endl;
    std::cout << "FEN: " << fen << std::endl;
    std::cout << std::endl;
    
    std::cout << "Expected board (from FEN):" << std::endl;
    std::cout << "8 | r . . . k . . r" << std::endl;
    std::cout << "7 | p . p p q p b ." << std::endl;
    std::cout << "6 | b n . . p n p ." << std::endl;
    std::cout << "5 | . . . P N . . ." << std::endl;
    std::cout << "4 | . p . . P . . ." << std::endl;
    std::cout << "3 | . . N . . Q . p" << std::endl;
    std::cout << "2 | P P P B B P P P" << std::endl;
    std::cout << "1 | R . . . K . . R" << std::endl;
    std::cout << "  +----------------" << std::endl;
    std::cout << "    a b c d e f g h" << std::endl;
    std::cout << std::endl;
    
    std::cout << "UltraEngine thinks board is:" << std::endl;
    std::cout << "  a b c d e f g h" << std::endl;
    std::cout << "  ----------------" << std::endl;
    
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
    
    // Check specific important squares
    std::cout << "=== KEY SQUARE VERIFICATION ===" << std::endl;
    struct {const char* name; int square; char expected;} key_squares[] = {
        {"a1", 0, 'R'}, {"e1", 4, 'K'}, {"h1", 7, 'R'},
        {"a2", 8, 'P'}, {"b2", 9, 'P'}, {"c2", 10, 'P'}, {"d2", 11, 'B'}, {"e2", 12, 'B'}, {"f2", 13, 'P'}, {"g2", 14, 'P'}, {"h2", 15, 'P'},
        {"c3", 18, 'N'}, {"f3", 21, 'Q'}, {"h3", 23, 'p'},
        {"b4", 25, 'p'}, {"e4", 28, 'P'},
        {"d5", 35, 'P'}, {"e5", 36, 'N'},
        {"a6", 40, 'b'}, {"b6", 41, 'n'}, {"e6", 44, 'p'}, {"f6", 45, 'n'}, {"g6", 46, 'p'},
        {"a7", 48, 'p'}, {"c7", 50, 'p'}, {"d7", 51, 'p'}, {"e7", 52, 'q'}, {"f7", 53, 'p'}, {"g7", 54, 'b'},
        {"a8", 56, 'r'}, {"e8", 60, 'k'}, {"h8", 63, 'r'}
    };
    
    bool all_correct = true;
    for (const auto& sq : key_squares) {
        int piece_info = pos.get_piece_at(sq.square);
        char actual = piece_to_char(piece_info);
        bool correct = (actual == sq.expected);
        
        std::cout << sq.name << " (square " << std::setw(2) << sq.square << "): "
                  << "expected '" << sq.expected << "', got '" << actual << "'"
                  << (correct ? " ✓" : " ✗") << std::endl;
        
        if (!correct) all_correct = false;
    }
    
    std::cout << std::endl;
    std::cout << "Board parsing: " << (all_correct ? "✓ CORRECT" : "✗ INCORRECT") << std::endl;
    
    return 0;
}