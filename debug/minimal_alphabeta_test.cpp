#include <iostream>
#include <chrono>
#include "position.hpp" 
#include "movegen_enhanced.hpp"
#include "search.hpp"
#include "evaluation.hpp"
#include "init.hpp"

// Minimal alpha-beta that mimics the full engine more closely
int minimal_alpha_beta(Position& pos, int depth, int alpha, int beta) {
    // Match the full engine's structure exactly
    if (depth <= 0) {
        // Simple evaluation instead of quiescence
        return Huginn::HybridEvaluator::evaluate(pos);
    }
    
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    
    if (legal_moves.count == 0) {
        // Check for mate/stalemate like the full engine
        int king_sq = pos.king_sq[int(pos.side_to_move)];
        if (king_sq >= 0 && SqAttacked(king_sq, pos, !pos.side_to_move)) {
            return -30000 + depth; // Mate score
        } else {
            return 0; // Stalemate
        }
    }
    
    // NO move ordering - this was working
    
    for (int i = 0; i < legal_moves.count; ++i) {
        if (pos.MakeMove(legal_moves.moves[i]) == 1) {
            int score = -minimal_alpha_beta(pos, depth - 1, -beta, -alpha);
            pos.TakeMove();
            
            if (score >= beta) {
                return beta;
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
    
    std::cout << "=== Minimal Alpha-Beta with Evaluation Test ===" << std::endl;
    std::cout << "Starting position: " << pos.to_fen() << std::endl;
    
    try {
        for (int depth = 1; depth <= 4; ++depth) {
            std::cout << "Testing depth " << depth << "..." << std::endl;
            
            auto start = std::chrono::steady_clock::now();
            int score = minimal_alpha_beta(pos, depth, -50000, 50000);
            auto end = std::chrono::steady_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            std::cout << "  Depth " << depth << ": score " << score 
                      << " in " << duration.count() << "ms ✓" << std::endl;
        }
        
        std::cout << "✓ SUCCESS: Minimal alpha-beta with evaluation works!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ ERROR: Exception caught: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "✗ ERROR: Unknown exception caught" << std::endl; 
        return 1;
    }
    
    return 0;
}
