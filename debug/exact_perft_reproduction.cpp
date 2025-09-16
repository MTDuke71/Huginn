#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/movegen_enhanced.hpp"
#include <iostream>

// Exact copy of perft function from perft_huginn2.cpp
uint64_t perft_exact_copy(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    uint64_t nodes = 0;
    S_MOVELIST move_list;
    
    // Generate all legal moves using bitboard implementation
    generate_legal_moves_enhanced(pos, move_list);
    
    for (int i = 0; i < move_list.count; ++i) {
        const S_MOVE& move = move_list.moves[i];
        
        // Make the move
        if (pos.MakeMove(move) == 1) {
            nodes += perft_exact_copy(pos, depth - 1);
            pos.TakeMove(); // Unmake the move
        }
    }
    
    return nodes;
}

int main() {
    std::cout << "Exact Perft Reproduction Test" << std::endl;
    std::cout << "=============================" << std::endl << std::endl;
    
    // Add the missing initialization!
    std::cout << "Calling Huginn::init()..." << std::endl;
    Huginn::init();
    
    // Exact same initialization as perft_huginn2.cpp
    Position pos;
    
    std::cout << "Setting up starting position..." << std::endl;
    pos.set_startpos();
    
    // Check bitboards like perft_huginn2 does
    uint64_t white_knights = pos.piece_bitboards[int(Color::White)][int(PieceType::Knight)];
    uint64_t black_knights = pos.piece_bitboards[int(Color::Black)][int(PieceType::Knight)];
    std::cout << "Debug: White knights bitboard: 0x" << std::hex << white_knights << std::dec << std::endl;
    std::cout << "Debug: Black knights bitboard: 0x" << std::hex << black_knights << std::dec << std::endl;
    
    // Test depth 1 - exact same as perft_huginn2
    std::cout << "\nTesting depth 1..." << std::endl;
    uint64_t result = perft_exact_copy(pos, 1);
    
    std::cout << "Depth 1: " << result << " nodes (expected: 20)" << std::endl;
    
    if (result == 20) {
        std::cout << "✅ PASS - This matches expected result" << std::endl;
    } else {
        std::cout << "❌ FAIL - This reproduces the perft_huginn2 bug!" << std::endl;
        
        // Let's see what moves we get
        S_MOVELIST move_list;
        generate_legal_moves_enhanced(pos, move_list);
        std::cout << "Generated moves: " << move_list.count << std::endl;
    }
    
    return 0;
}