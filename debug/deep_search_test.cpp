#include "../src/minimal_search.hpp"
#include "../src/position.hpp"
#include "../src/evaluation.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <vector>
#include <chrono>

using namespace Huginn;

struct TestPosition {
    std::string fen;
    std::string description;
    std::vector<std::string> good_moves;
    std::vector<std::string> bad_moves;
};

int main() {
    init();
    
    std::cout << "=== DEEP SEARCH EVALUATION TEST ===" << std::endl;
    
    MinimalEngine engine;
    
    // Test positions to evaluate our improvements
    std::vector<TestPosition> positions = {
        {
            "rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 1",
            "After 1.c4 - Black should develop or control center",
            {"d7d5", "e7e5", "g8f6", "b8c6"},
            {"b7b6", "a7a6", "h7h6", "g7g6"}
        },
        {
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
            "After 1.e4 - Black should respond centrally",
            {"e7e5", "d7d5", "g8f6", "b8c6"},
            {"a7a6", "h7h6", "b7b6", "g7g6"}
        },
        {
            "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
            "After 1.e4 e5 - White should develop",
            {"g1f3", "b1c3", "f1c4", "d2d3"},
            {"a2a3", "h2h3", "g2g3", "b2b3"}
        }
    };
    
    for (const auto& test : positions) {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "Testing: " << test.description << std::endl;
        std::cout << "FEN: " << test.fen << std::endl;
        
        Position pos;
        pos.set_from_fen(test.fen);
        
        // Test multiple depths
        for (int depth = 6; depth <= 12; depth += 2) {
            std::cout << "\n--- Depth " << depth << " ---" << std::endl;
            
            SearchInfo info;
            info.max_depth = depth;
            info.stopped = false;
            info.fh = 0;
            info.fhf = 0;
            
            auto start = std::chrono::high_resolution_clock::now();
            S_MOVE best_move = engine.searchPosition(pos, info);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::string best_move_str = engine.move_to_uci(best_move);
            
            std::cout << "Best: " << best_move_str 
                     << " | Nodes: " << info.nodes 
                     << " | Time: " << duration.count() << "ms";
            
            // Check if move is good or bad
            bool is_good = std::find(test.good_moves.begin(), test.good_moves.end(), best_move_str) != test.good_moves.end();
            bool is_bad = std::find(test.bad_moves.begin(), test.bad_moves.end(), best_move_str) != test.bad_moves.end();
            
            if (is_good) {
                std::cout << " ✅ GOOD";
            } else if (is_bad) {
                std::cout << " ❌ BAD";
            } else {
                std::cout << " ⚪ NEUTRAL";
            }
            
            // Move ordering efficiency
            if (info.fh > 0) {
                double efficiency = (double)info.fhf / (double)info.fh * 100.0;
                std::cout << " | Ordering: " << std::fixed << std::setprecision(1) << efficiency << "%";
            }
            
            std::cout << std::endl;
            
            // Stop if search is taking too long
            if (duration.count() > 10000) { // 10 seconds
                std::cout << "Stopping deeper search due to time limit" << std::endl;
                break;
            }
        }
    }
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "Deep search evaluation test complete!" << std::endl;
    
    return 0;
}
