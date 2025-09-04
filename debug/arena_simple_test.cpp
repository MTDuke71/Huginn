#include "../src/search.hpp"
#include "../src/position.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <chrono>

using namespace Huginn;

int main() {
    init();
    
    std::cout << "=== ARENA-EQUIVALENT SEARCH TEST ===" << std::endl;
    std::cout << "Testing SimpleEngine with time-based search (like Arena uses)" << std::endl << std::endl;
    
    SimpleEngine engine;
    
    // Test position after 1.c4
    std::cout << "Testing: After 1.c4 - Black to move" << std::endl;
    std::cout << "FEN: rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 1" << std::endl << std::endl;
    
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 1");
    
    // Test different time limits
    std::vector<int> time_limits = {1000, 3000, 5000}; // 1s, 3s, 5s
    
    for (int time_ms : time_limits) {
        std::cout << "--- " << time_ms << "ms time limit ---" << std::endl;
        
        SearchLimits limits;
        limits.max_depth = 0;        // Unlimited depth (like Arena)
        limits.max_time_ms = time_ms; // Time-based search
        limits.infinite = false;
        
        auto start = std::chrono::high_resolution_clock::now();
        S_MOVE best_move = engine.search(pos, limits);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto actual_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        const SearchStats& stats = engine.get_stats();
        
        std::cout << "Best move: " << SimpleEngine::move_to_uci(best_move) << std::endl;
        std::cout << "Max depth reached: " << stats.max_depth_reached << std::endl;
        std::cout << "Nodes searched: " << stats.nodes_searched << std::endl;
        std::cout << "Actual time: " << actual_time.count() << "ms" << std::endl;
        
        if (actual_time.count() > 0) {
            std::cout << "Speed: " << (stats.nodes_searched * 1000 / actual_time.count()) 
                     << " nodes/second" << std::endl;
        }
        std::cout << std::endl;
        
        engine.reset(); // Reset for next search
        
        // If reaching very high depths, stop testing longer times
        if (stats.max_depth_reached > 20) {
            std::cout << "Stopping further tests - already reaching depth " << stats.max_depth_reached << std::endl;
            break;
        }
    }
    
    std::cout << "Arena-equivalent search test complete!" << std::endl;
    std::cout << "This should match what Arena sees when using Huginn." << std::endl;
    
    return 0;
}
