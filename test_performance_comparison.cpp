#include <iostream>
#include <chrono>
#include "src/position.hpp"
#include "src/movegen.hpp"
#include "src/init.hpp"

uint64_t perft_single_arg(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    MoveList moves;
    generate_legal_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.v) {
        pos.make_move_with_undo(move);  // Single argument
        nodes += perft_single_arg(pos, depth - 1);
        pos.undo_move();                // Automatic undo
    }
    return nodes;
}

int main() {
    Huginn::init();
    
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    pos.set_from_fen(kiwipete_fen);
    
    // Test single-argument system performance
    std::cout << "=== Testing Single-Argument System Performance ===" << std::endl;
    
    MoveList moves;
    generate_legal_moves(pos, moves);
    
    std::cout << "Testing first 5 moves at depth 2..." << std::endl;
    
    for (int i = 0; i < 5 && i < moves.size(); i++) {
        const auto& move = moves[i];
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        pos.make_move_with_undo(move);
        uint64_t nodes = perft_single_arg(pos, 2);
        pos.undo_move();
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "Move " << (i+1) << ": " << nodes << " nodes in " << duration.count() << "ms" << std::endl;
    }
    
    return 0;
}
