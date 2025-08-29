#include <iostream>
#include <chrono>
#include "position.hpp" 
#include "movegen_enhanced.hpp"
#include "search.hpp"
#include "init.hpp"

int main() {
    // Initialize the chess engine
    Huginn::init();
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "=== Full Search Engine Test ===" << std::endl;
    std::cout << "Starting position: " << pos.to_fen() << std::endl;
    
    // Create the search engine
    std::cout << "Creating engine instance..." << std::endl;
    
    // Test with minimal search limits
    Huginn::SearchLimits limits;
    limits.max_depth = 4;  // Same depth that worked in our other tests
    limits.max_time_ms = 10000;
    limits.max_nodes = UINT64_MAX;
    limits.infinite = false;
    
    std::cout << "Testing full search engine at depth " << limits.max_depth << "..." << std::endl;
    
    try {
        std::cout << "  Creating search engine..." << std::endl;
        Huginn::SimpleEngine engine;
        
        std::cout << "  Position before search: ply=" << pos.ply << ", move_history size=" << pos.move_history.size() << std::endl;
        
        std::cout << "  Starting search..." << std::endl;
        auto start = std::chrono::steady_clock::now();
        S_MOVE best_move = engine.search(pos, limits);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "  SUCCESS: Found move " << std::hex << best_move.move 
                  << " (score " << std::dec << best_move.score << ")"
                  << " in " << duration.count() << "ms ✓" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "  ERROR: Exception caught: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "  ERROR: Unknown exception caught" << std::endl; 
        return 1;
    }
    
    std::cout << "=== Full search engine test passed! ===" << std::endl;
    return 0;
}
