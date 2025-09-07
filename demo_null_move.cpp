#include <iostream>
#include "src/position.hpp"
#include "src/minimal_search.hpp"

using namespace Huginn;

int main() {
    MinimalEngine engine;
    
    std::cout << "=== Null Move Pruning Demonstration ===" << std::endl;
    std::cout << "This test will show null move cutoff statistics in the search output." << std::endl;
    std::cout << "Look for 'nullcut' in the info lines - this shows how many positions" << std::endl;
    std::cout << "were pruned by null move, speeding up the search significantly." << std::endl;
    std::cout << std::endl;
    
    // Test position: A middlegame position with many pieces - good for demonstrating null move
    std::string fen = "r1bq1rk1/pp2nppp/2n1p3/3pP3/2PP4/2N1BN2/PP3PPP/R2QK2R w KQ - 0 10";
    Position pos;
    pos.set_from_fen(fen);
    
    std::cout << "Position: " << fen << std::endl;
    std::cout << "Searching to depth 6..." << std::endl;
    std::cout << std::endl;
    
    MinimalLimits limits;
    limits.max_depth = 6;  // Deep enough to see null move pruning effects
    limits.max_time_ms = 10000;  // 10 second limit
    
    S_MOVE best_move = engine.search(pos, limits);
    
    std::cout << std::endl;
    std::cout << "Search completed!" << std::endl;
    std::cout << "Best move: " << engine.move_to_uci(best_move) << std::endl;
    std::cout << std::endl;
    std::cout << "Analysis of output:" << std::endl;
    std::cout << "- 'nodes': Total positions evaluated" << std::endl;
    std::cout << "- 'nullcut': Positions pruned by null move" << std::endl;
    std::cout << "- Higher nullcut values indicate more effective pruning" << std::endl;
    std::cout << "- This reduces search time while maintaining move quality" << std::endl;
    
    return 0;
}
