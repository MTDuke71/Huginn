#include "../src/position.hpp"
#include "../src/bitboard.hpp"
#include "../src/init.hpp"
#include "../src/bitboard_movegen.hpp"
#include "../src/pawn_optimizations.hpp"
#include <iostream>
#include <bitset>

void print_bitboard(uint64_t bb, const std::string& name) {
    std::cout << name << ":\n";
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << " ";
        for (int file = 0; file < 8; file++) {
            int sq64 = rank * 8 + file;
            if (bb & (1ULL << sq64)) {
                std::cout << "1 ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "  a b c d e f g h\n\n";
}

int main() {
    std::cout << "Bishop Attack Debug\n";
    std::cout << "===================\n";
    
    Huginn::init();
    
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Make moves: b2b3 a7a6
    S_MOVE move1 = S_MOVE(22, 33, PieceType::None, false, false, PieceType::None, false);  // b2-b3
    pos.make_move_with_undo(move1);
    
    S_MOVE move2 = S_MOVE(91, 81, PieceType::None, false, false, PieceType::None, false);  // a7-a6
    pos.make_move_with_undo(move2);
    
    std::cout << "Position after b2b3 a7a6:\n";
    // pos.print_board();  // Function not available - comment out
    std::cout << "\n";
    
    // Check c1 bishop (square 2 in 64-bit, square 23 in 120-bit)
    int c1_sq64 = 2;  // c1 in 64-bit indexing
    int c1_sq120 = 23; // c1 in 120-bit indexing
    
    std::cout << "Analyzing bishop on c1 (sq64=" << c1_sq64 << ", sq120=" << c1_sq120 << ")\n";
    
    // Print current occupancy
    uint64_t occupied = pos.occupied_bitboard;
    std::cout << "Current occupancy (all pieces):\n";
    print_bitboard(occupied, "Occupied squares");
    
    // Print White pieces
    uint64_t white_pieces = pos.color_bitboards[int(Color::White)];
    std::cout << "White pieces:\n";
    print_bitboard(white_pieces, "White pieces");
    
    // Print bishop bitboard
    uint64_t bishops = pos.piece_bitboards[int(Color::White)][int(PieceType::Bishop)];
    std::cout << "White bishops:\n";
    print_bitboard(bishops, "White bishops");
    
    // Get bishop attacks from c1
    uint64_t bishop_attacks_raw = bishop_attacks(c1_sq64, occupied);
    std::cout << "Raw bishop attacks from c1:\n";
    print_bitboard(bishop_attacks_raw, "Raw bishop attacks");
    
    // Filter out own pieces
    uint64_t valid_attacks = bishop_attacks_raw & ~white_pieces;
    std::cout << "Valid bishop attacks (excluding own pieces):\n";
    print_bitboard(valid_attacks, "Valid attacks");
    
    // Check specific squares a3 and b2
    int a3_sq64 = 16;  // a3
    int b2_sq64 = 9;   // b2
    
    std::cout << "Checking specific squares:\n";
    std::cout << "a3 (sq64=" << a3_sq64 << "): " << ((valid_attacks & (1ULL << a3_sq64)) ? "PRESENT" : "MISSING") << "\n";
    std::cout << "b2 (sq64=" << b2_sq64 << "): " << ((valid_attacks & (1ULL << b2_sq64)) ? "PRESENT" : "MISSING") << "\n";
    
    // Check what's on b2 currently
    std::cout << "\nSquare analysis:\n";
    std::cout << "b2 (sq120=22): " << to_char(pos.at(22)) << "\n";
    std::cout << "a3 (sq120=81): " << to_char(pos.at(81)) << "\n";
    
    return 0;
}