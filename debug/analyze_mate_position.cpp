#include "../src/minimal_search.hpp"
#include "../src/position.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <vector>

using namespace Huginn;

// Analyze the critical position where Huginn walked into mate
int main() {
    init();
    
    Position pos;
    MinimalEngine engine;
    
    // Set up the critical position from the game log
    std::string fen_moves = "d2d4 g8f6 g1f3 e7e6 c1g5 h7h6 g5f6 d8f6 e2e4 d7d6 b1c3 b8d7 f1b5 f8e7 d1d3 c7c6 b5c6 b7c6 d3c4 c8b7 c4b4 e8c8 b4a5 d6d5 a5a7 d5e4 c3e4 f6g6 e4c3 g6g2 h1g1 g2f3 g1g7 f3h1 e1d2 h1a1 g7f7 e7c5 a7a4 d7b6 a4b3 d8d4 d2e2 b7a6 e2f3 a1h1 f3g3 h8g8 f7g7 g8g7";
    
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Apply all moves to reach the critical position
    std::istringstream moves_stream(fen_moves);
    std::string move_str;
    
    while (moves_stream >> move_str) {
        S_MOVELIST move_list;
        generate_legal_moves_enhanced(pos, move_list);
        
        bool found = false;
        for (int i = 0; i < move_list.count; ++i) {
            if (engine.move_to_uci(move_list.moves[i]) == move_str) {
                pos.MakeMove(move_list.moves[i]);
                found = true;
                break;
            }
        }
        
        if (!found) {
            std::cout << "Failed to apply move: " << move_str << std::endl;
            return 1;
        }
    }
    
    std::cout << "=== CRITICAL POSITION ANALYSIS ===\n";
    std::cout << "Position reached after all moves\n";
    std::cout << "White to move (Huginn)\n\n";
    
    // Analyze the position with different depths
    std::cout << "=== SEARCH ANALYSIS ===\n";
    
    MinimalLimits limits;
    limits.max_time_ms = 5000;
    limits.infinite = false;
    
    for (int depth = 1; depth <= 8; depth++) {
        limits.max_depth = depth;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        S_MOVE best_move = engine.search(pos, limits);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        int eval = engine.evaluate(pos);
        auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        
        std::cout << "Depth " << depth << ": " 
                  << engine.move_to_uci(best_move) 
                  << " (eval: " << eval << "cp, time: " << time_ms << "ms)\n";
    }
    
    // Generate and analyze all legal moves
    std::cout << "\n=== ALL LEGAL MOVES ===\n";
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    
    for (int i = 0; i < move_list.count; ++i) {
        S_MOVE move = move_list.moves[i];
        
        // Make the move and evaluate
        pos.MakeMove(move);
        int eval = -engine.evaluate(pos);  // Negate for opponent's perspective
        pos.TakeMove();
        
        std::cout << engine.move_to_uci(move) << ": " << eval << "cp\n";
    }
    
    return 0;
}
