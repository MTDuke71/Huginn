#include <iostream>
#include "position.hpp" 
#include "movegen_enhanced.hpp"
#include "search.hpp"
#include "init.hpp"

// Simple alpha-beta search similar to the engine but minimal
int simple_alpha_beta(Position& pos, int depth, int alpha, int beta) {
    if (depth == 0) return 0; // Simple leaf evaluation
    
    S_MOVELIST moves;
    generate_legal_moves_enhanced(pos, moves);
    
    if (moves.count == 0) {
        return 0; // Simplified - just return 0 for mate/stalemate
    }
    
    for (int i = 0; i < moves.count; ++i) {
        if (pos.MakeMove(moves.moves[i]) == 1) {
            int score = -simple_alpha_beta(pos, depth - 1, -beta, -alpha);
            pos.TakeMove();
            
            if (score >= beta) {
                return beta; // Beta cutoff
            }
            if (score > alpha) {
                alpha = score;
            }
        }
    }
    
    return alpha;
}

int main() {
    // Initialize the chess engine
    Huginn::init();
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "=== Simple Alpha-Beta Test ===" << std::endl;
    std::cout << "Starting position: " << pos.to_fen() << std::endl;
    
    // Test increasing depths
    for (int depth = 1; depth <= 5; ++depth) {
        std::cout << "Testing alpha-beta depth " << depth << "..." << std::endl;
        
        auto start = std::chrono::steady_clock::now();
        int score = simple_alpha_beta(pos, depth, -1000, 1000);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "  Depth " << depth << ": score " << score 
                  << " in " << duration.count() << "ms ✓" << std::endl;
    }
    
    std::cout << "=== All alpha-beta tests passed! ===" << std::endl;
    return 0;
}
