#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/init.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    init(); // Initialize engine subsystems
    
    std::cout << "=== Debug Test: FEN Loading ===\n";
    
    MinimalEngine engine;
    Position pos;
    
    // Test FEN loading first
    std::cout << "Testing FEN loading...\n";
    std::string test_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if (!pos.set_from_fen(test_fen)) {
        std::cout << "ERROR: Failed to load starting position FEN\n";
        return 1;
    }
    std::cout << "✅ Starting position loaded successfully\n";
    
    // Test WAC.1 position
    std::string wac_fen = "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1";
    if (!pos.set_from_fen(wac_fen)) {
        std::cout << "ERROR: Failed to load WAC.1 position FEN\n";
        return 1;
    }
    std::cout << "✅ WAC.1 position loaded successfully\n";
    
    // Test engine initialization
    std::cout << "Testing engine initialization...\n";
    
    // Test search with incremental depths
    for (int depth = 1; depth <= 4; depth++) {
        SearchInfo info;
        info.max_depth = depth;
        info.depth_only = true;  // Bypass time management
        info.stopped = false;
        info.fh = 0;
        info.fhf = 0;
        
        std::cout << "Testing depth " << depth << " search...\n";
        try {
            S_MOVE best_move = engine.searchPosition(pos, info);
            std::cout << "✅ Depth " << depth << " completed successfully\n";
            std::cout << "Best move: " << engine.move_to_uci(best_move) << std::endl;
            std::cout << "Nodes: " << info.nodes << std::endl;
        } catch (...) {
            std::cout << "❌ ERROR: Depth " << depth << " search failed with exception\n";
            return 1;
        }
    }
    
    std::cout << "=== Debug Test Complete ===\n";
    return 0;
}
