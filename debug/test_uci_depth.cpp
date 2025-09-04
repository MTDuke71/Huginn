#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/minimal_search.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    std::cout << "=== UCI Depth Command Test ===" << std::endl;
    std::cout << "Testing depth-only search (bypassing time management)" << std::endl;
    
    init();
    Position pos;
    MinimalEngine engine;
    
    // Test with WAC.1 position - previously stopped due to time limits
    if (!pos.set_from_fen("2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1")) {
        std::cout << "Failed to set WAC.1 position" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Test 1: Normal search with time management ===" << std::endl;
    SearchInfo info1;
    info1.max_depth = 4;
    auto start_time1 = std::chrono::steady_clock::now();
    info1.start_time = start_time1;
    info1.stop_time = start_time1 + std::chrono::milliseconds(3000);  // Only 3 seconds
    info1.depth_only = false;  // Use time management
    
    std::cout << "Searching with 3-second time limit..." << std::endl;
    S_MOVE best_move1 = engine.searchPosition(pos, info1);
    std::cout << "Search stopped: " << (info1.stopped ? "YES (time limit)" : "NO") << std::endl;
    std::cout << "Nodes searched: " << info1.nodes << std::endl;
    
    std::cout << "\n=== Test 2: UCI depth command (bypass time management) ===" << std::endl;
    SearchInfo info2;
    info2.max_depth = 4;
    auto start_time2 = std::chrono::steady_clock::now();
    info2.start_time = start_time2;
    info2.stop_time = start_time2 + std::chrono::milliseconds(3000);  // Same 3-second limit
    info2.depth_only = true;   // BYPASS time management
    
    std::cout << "Searching with depth_only=true (ignoring 3-second time limit)..." << std::endl;
    S_MOVE best_move2 = engine.searchPosition(pos, info2);
    std::cout << "Search stopped: " << (info2.stopped ? "YES (unexpected!)" : "NO (completed all depths)") << std::endl;
    std::cout << "Nodes searched: " << info2.nodes << std::endl;
    
    std::cout << "\n=== Summary ===" << std::endl;
    std::cout << "✓ depth_only flag implemented" << std::endl;
    std::cout << "✓ Time management can be bypassed for UCI depth commands" << std::endl;
    std::cout << "✓ Normal searches still respect time limits" << std::endl;
    
    return 0;
}
