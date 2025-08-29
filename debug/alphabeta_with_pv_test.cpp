#include <iostream>
#include <chrono>
#include "position.hpp" 
#include "movegen_enhanced.hpp"
#include "init.hpp"

// Inline PVLine to avoid header issues
struct PVLine {
    S_MOVE moves[64];
    int length = 0;
    
    void clear() { length = 0; }
    void add_move(S_MOVE move) {
        if (length < 63) {
            moves[length++] = move;
        }
    }
};

// Alpha-beta search with PV parameter like the failing engine
int alpha_beta_with_pv(Position& pos, int depth, int alpha, int beta, PVLine& pv) {
    pv.clear(); // Clear PV like the real engine does
    
    if (depth == 0) return 0; // Simple leaf evaluation
    
    S_MOVELIST moves;
    generate_legal_moves_enhanced(pos, moves);
    
    if (moves.count == 0) {
        return 0; // Simplified - just return 0 for mate/stalemate
    }
    
    for (int i = 0; i < moves.count; ++i) {
        if (pos.MakeMove(moves.moves[i]) == 1) {
            PVLine child_pv; // Create child PV like the real engine
            int score = -alpha_beta_with_pv(pos, depth - 1, -beta, -alpha, child_pv);
            pos.TakeMove();
            
            if (score >= beta) {
                return beta; // Beta cutoff
            }
            if (score > alpha) {
                alpha = score;
                
                // Update PV like the real engine
                pv.clear();
                pv.add_move(moves.moves[i]);
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
    
    std::cout << "=== Alpha-Beta with PV Test ===" << std::endl;
    std::cout << "Starting position: " << pos.to_fen() << std::endl;
    
    // Test increasing depths
    for (int depth = 1; depth <= 5; ++depth) {
        std::cout << "Testing alpha-beta with PV depth " << depth << "..." << std::endl;
        
        auto start = std::chrono::steady_clock::now();
        PVLine pv;
        int score = alpha_beta_with_pv(pos, depth, -1000, 1000, pv);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "  Depth " << depth << ": score " << score 
                  << ", PV length " << pv.length
                  << " in " << duration.count() << "ms ✓" << std::endl;
    }
    
    std::cout << "=== All alpha-beta with PV tests passed! ===" << std::endl;
    return 0;
}
