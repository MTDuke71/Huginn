#include <iostream>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "search.hpp"
#include "init.hpp"

// Simple perft using VICE
uint64_t perft_vice(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    S_MOVELIST moves;
    generate_legal_moves_enhanced(pos, moves);
    
    uint64_t nodes = 0;
    for (int i = 0; i < moves.count; ++i) {
        if (pos.MakeMove(moves.moves[i]) == 1) {
            nodes += perft_vice(pos, depth - 1);
            pos.TakeMove();
        }
    }
    return nodes;
}

int main() {
    // Initialize the chess engine
    Huginn::init();
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "=== Simple VICE Perft Test ===" << std::endl;
    std::cout << "Starting position: " << pos.to_fen() << std::endl;
    
    // Test increasing depths
    for (int depth = 1; depth <= 4; ++depth) {
        std::cout << "Testing perft(" << depth << ")..." << std::endl;
        
        auto start = std::chrono::steady_clock::now();
        uint64_t nodes = perft_vice(pos, depth);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Expected results for starting position
        uint64_t expected[] = {0, 20, 400, 8902, 197281};
        
        std::cout << "  Depth " << depth << ": " << nodes << " nodes in " 
                  << duration.count() << "ms";
        
        if (depth <= 4) {
            if (nodes == expected[depth]) {
                std::cout << " ✓ CORRECT" << std::endl;
            } else {
                std::cout << " ✗ WRONG (expected " << expected[depth] << ")" << std::endl;
                return 1;
            }
        } else {
            std::cout << std::endl;
        }
    }
    
    std::cout << "=== All perft tests passed! ===" << std::endl;
    return 0;
}
