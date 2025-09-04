#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/minimal_search.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    std::cout << "=== Debug WAC.1 Search Issue ===" << std::endl;
    
    init();
    Position pos;
    MinimalEngine engine;
    
    // Test with WAC.1 position
    if (!pos.set_from_fen("2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1")) {
        std::cout << "Failed to set WAC.1 position" << std::endl;
        return 1;
    }
    
    SearchInfo info;
    info.max_depth = 3;  // Lower depth to see if it completes
    auto start_time = std::chrono::steady_clock::now();
    info.start_time = start_time;
    info.stop_time = start_time + std::chrono::milliseconds(10000);  // 10 seconds
    info.infinite = false;
    
    std::cout << "Starting search with max_depth=" << info.max_depth << std::endl;
    std::cout << "Time limit: 10 seconds" << std::endl;
    
    S_MOVE best_move = engine.searchPosition(pos, info);
    
    std::cout << "\nSearch completed." << std::endl;
    std::cout << "Final best move: " << engine.move_to_uci(best_move) << std::endl;
    std::cout << "Total nodes: " << info.nodes << std::endl;
    std::cout << "Search stopped: " << (info.stopped ? "YES" : "NO") << std::endl;
    std::cout << "Search quit: " << (info.quit ? "YES" : "NO") << std::endl;
    
    return 0;
}
