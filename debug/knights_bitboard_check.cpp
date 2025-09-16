#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/bitboard.hpp"
#include <iostream>
#include <iomanip>

void print_bitboard(Bitboard bb, const std::string& name) {
    std::cout << name << ": 0x" << std::hex << bb << std::dec << std::endl;
    
    // Print the board layout
    std::cout << "Board layout:" << std::endl;
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            int sq64 = rank * 8 + file;
            if (bb & (1ULL << sq64)) {
                std::cout << "N ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "  a b c d e f g h" << std::endl << std::endl;
}

int main() {
    std::cout << "Knights Bitboard Verification" << std::endl;
    std::cout << "============================" << std::endl << std::endl;
    
    Huginn::init();
    Position pos;
    pos.set_startpos();
    
    // Get the knight bitboards
    Bitboard white_knights = pos.get_piece_bitboard(Color::White, PieceType::Knight);
    Bitboard black_knights = pos.get_piece_bitboard(Color::Black, PieceType::Knight);
    
    std::cout << "Starting position knights:" << std::endl;
    print_bitboard(white_knights, "White knights");
    print_bitboard(black_knights, "Black knights");
    
    // Calculate expected values manually
    // White knights: b1 (sq64=1) and g1 (sq64=6)
    Bitboard expected_white = (1ULL << 1) | (1ULL << 6);
    // Black knights: b8 (sq64=57) and g8 (sq64=62)
    Bitboard expected_black = (1ULL << 57) | (1ULL << 62);
    
    std::cout << "Expected values:" << std::endl;
    print_bitboard(expected_white, "Expected white knights");
    print_bitboard(expected_black, "Expected black knights");
    
    // Verify correctness
    std::cout << "Verification:" << std::endl;
    std::cout << "White knights correct: " << (white_knights == expected_white ? "YES" : "NO") << std::endl;
    std::cout << "Black knights correct: " << (black_knights == expected_black ? "YES" : "NO") << std::endl;
    
    // Also check individual squares
    std::cout << std::endl << "Individual square checks:" << std::endl;
    std::cout << "b1 (sq64=1): " << (white_knights & (1ULL << 1) ? "SET" : "CLEAR") << std::endl;
    std::cout << "g1 (sq64=6): " << (white_knights & (1ULL << 6) ? "SET" : "CLEAR") << std::endl;
    std::cout << "b8 (sq64=57): " << (black_knights & (1ULL << 57) ? "SET" : "CLEAR") << std::endl;
    std::cout << "g8 (sq64=62): " << (black_knights & (1ULL << 62) ? "SET" : "CLEAR") << std::endl;
    
    return 0;
}