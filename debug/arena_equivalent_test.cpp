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
    
    // Test the same positions but with time-based search like Arena
    std::vector<std::pair<std::string, std::string>> test_positions = {
        {
            "rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 1",
            "After 1.c4 - Black to move"
        },
        {
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", 
            "After 1.e4 - Black to move"
        },
        {
            "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
            "After 1.e4 e5 - White to move"
        }
    };
    
    for (const auto& test : test_positions) {
        std::cout << std::string(60, '=') << std::endl;
        std::cout << "Testing: " << test.second << std::endl;
        std::cout << "FEN: " << test.first << std::endl << std::endl;
        
        Position pos;
        pos.set_from_fen(test.first);
        
        // Test different time limits
        std::vector<int> time_limits = {1000, 5000, 10000}; // 1s, 5s, 10s
        
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
            std::cout << "Speed: " << (stats.nodes_searched * 1000 / std::max(1ULL, (uint64_t)actual_time.count())) 
                     << " nodes/second" << std::endl;
            
            const PVLine& pv = engine.get_pv();
            std::cout << "Principal variation: ";
            for (int i = 0; i < std::min(5, pv.length); ++i) {
                std::cout << SimpleEngine::move_to_uci(pv.moves[i]) << " ";
            }
            std::cout << std::endl << std::endl;
            
            engine.reset(); // Reset for next search
        }
    }
    
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Arena-equivalent search test complete!" << std::endl;
    std::cout << "This should match what Arena sees when using Huginn." << std::endl;
    
    return 0;
}
