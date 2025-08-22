#include <iostream>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "init.hpp"

// Perft function
static uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    MoveList list; 
    generate_legal_moves(pos, list);
    uint64_t nodes = 0;
    for (const auto& m : list.v) {
        pos.make_move_with_undo(m);
        nodes += perft(pos, depth - 1);
        pos.undo_move();
    }
    return nodes;
}

int main() {
    Huginn::init();
    
    std::cout << "=== Direct Perft Test of position after a2a4 ===" << std::endl;
    
    // Set up position after a2a4 directly
    Position pos;
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/2N2Q1p/1PPBBPPP/R3K2R b KQkq a3 0 1";
    
    if (!pos.set_from_fen(fen)) {
        std::cout << "Failed to parse FEN" << std::endl;
        return 1;
    }
    
    std::cout << "FEN: " << fen << std::endl;
    
    // Test perft depth 1
    uint64_t nodes = perft(pos, 1);
    std::cout << "Perft(1) = " << nodes << std::endl;
    std::cout << "Expected: 44" << std::endl;
    std::cout << "Difference: " << (int64_t(nodes) - 44) << std::endl;
    
    if (nodes == 44) {
        std::cout << "\n✓ CORRECT! Your engine generates the right number of moves." << std::endl;
        std::cout << "The earlier perft(2) difference was likely from a different issue." << std::endl;
    } else {
        std::cout << "\n✗ INCORRECT! Still missing moves." << std::endl;
    }
    
    return 0;
}
