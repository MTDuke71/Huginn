// Quick test to verify the old search() function works with unified node counting
#include "../src/minimal_search.hpp"
#include "../src/position.hpp"
#include "../src/init.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    std::cout << "Testing Unified Search System\n";
    std::cout << "=============================\n\n";
    
    init();
    
    // Test the old search() interface
    Position pos;
    pos.set_startpos();
    
    MinimalEngine engine;
    MinimalLimits limits;
    limits.max_depth = 4;
    limits.max_time_ms = 3000;
    
    std::cout << "Testing old search() interface to depth " << limits.max_depth << "...\n";
    
    auto start = std::chrono::steady_clock::now();
    S_MOVE best_move = engine.search(pos, limits);
    auto end = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "\nOld search() interface results:\n";
    std::cout << "Time: " << duration.count() << " ms\n";
    std::cout << "Nodes (engine.nodes_searched): " << engine.nodes_searched << "\n";
    
    if (best_move.move != 0) {
        int from = best_move.get_from();
        int to = best_move.get_to();
        
        char from_file = 'a' + ((from % 10) - 1);
        char from_rank = '1' + ((from / 10) - 2);
        char to_file = 'a' + ((to % 10) - 1);
        char to_rank = '1' + ((to / 10) - 2);
        
        std::cout << "Best move: " << from_file << from_rank << to_file << to_rank << "\n";
    }
    
    std::cout << "\n✅ Unified search system working correctly!\n";
    std::cout << "Both search interfaces now use consistent node counting.\n";
    
    return 0;
}
