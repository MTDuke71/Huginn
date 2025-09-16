#include <iostream>
#include "../src/position.hpp"
#include "../src/movegen_enhanced.hpp"
#include "../src/bitboard_movegen.hpp"
#include "../src/init.hpp"

int main() {
    std::cout << "Compilation Flag Debug\n";
    std::cout << "======================\n";
    
    // Check if BITBOARD_ENGINE is defined
    #ifdef BITBOARD_ENGINE
        std::cout << "✅ BITBOARD_ENGINE is DEFINED\n";
    #else
        std::cout << "❌ BITBOARD_ENGINE is NOT DEFINED\n";
    #endif
    
    // Initialize engine
    Huginn::init();
    
    // Set up starting position
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "\nTesting generate_all_moves() path:\n";
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    std::cout << "generate_all_moves() returned: " << moves.count << " moves\n";
    
    std::cout << "\nTesting generate_legal_moves_enhanced() path:\n";
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    std::cout << "generate_legal_moves_enhanced() returned: " << legal_moves.count << " moves\n";
    
    return 0;
}