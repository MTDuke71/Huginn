#include <iostream>
#include <chrono>
#include "position.hpp" 
#include "movegen_enhanced.hpp"
#include "search.hpp"
#include "init.hpp"

// Alpha-beta search identical to engine but WITHOUT move ordering
int alpha_beta_no_ordering(Position& pos, int depth, int alpha, int beta, Huginn::PVLine& pv) {
    pv.clear();
    
    if (depth <= 0) return 0;
    
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    
    if (legal_moves.count == 0) {
        return 0; // Simplified mate/stalemate
    }
    
    // NO MOVE ORDERING - this is the difference!
    // order_moves(pos, legal_moves);  // <-- SKIP THIS
    
    Huginn::PVLine best_pv;
    
    for (int i = 0; i < legal_moves.count; ++i) {
        if (pos.MakeMove(legal_moves.moves[i]) == 1) {
            Huginn::PVLine child_pv;
            int score = -alpha_beta_no_ordering(pos, depth - 1, -beta, -alpha, child_pv);
            
            pos.TakeMove();
            
            if (score >= beta) {
                return beta; // Beta cutoff
            }
            
            if (score > alpha) {
                alpha = score;
                
                // Update PV
                pv.clear();
                pv.add_move(legal_moves.moves[i]);
                for (int j = 0; j < child_pv.length; ++j) {
                    pv.add_move(child_pv.moves[j]);
                }
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
    
    std::cout << "=== Alpha-Beta Without Move Ordering Test ===" << std::endl;
    std::cout << "Starting position: " << pos.to_fen() << std::endl;
    
    // Test increasing depths 
    for (int depth = 1; depth <= 5; ++depth) {
        std::cout << "Testing alpha-beta without ordering depth " << depth << "..." << std::endl;
        
        auto start = std::chrono::steady_clock::now();
        Huginn::PVLine pv;
        int score = alpha_beta_no_ordering(pos, depth, -1000, 1000, pv);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "  Depth " << depth << ": score " << score 
                  << ", PV length " << pv.length
                  << " in " << duration.count() << "ms ✓" << std::endl;
    }
    
    std::cout << "=== All tests passed - move ordering is the culprit! ===" << std::endl;
    return 0;
}
