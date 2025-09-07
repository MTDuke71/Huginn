#include "src/init.hpp"
#include "src/minimal_search.hpp"
#include "src/position.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    init();
    
    std::cout << "=== Transposition Table Hit/Write Counter Demo ===" << std::endl;
    std::cout << std::endl;
    
    // Simple tactical position that should have some transpositions
    std::string fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    Position pos;
    pos.set_from_fen(fen);
    
    std::cout << "Position: Starting position after 1.e4" << std::endl;
    std::cout << "Searching to depth 4..." << std::endl;
    std::cout << std::endl;
    
    MinimalEngine engine;
    MinimalLimits limits;
    limits.max_depth = 4;  // Shorter depth to see clear results
    limits.max_time_ms = 10000;
    
    // Clear TT stats before search
    engine.tt_table.clear_stats();
    
    S_MOVE best_move = engine.search(pos, limits);
    
    std::cout << std::endl;
    std::cout << "Search completed! Best move: " << engine.move_to_uci(best_move) << std::endl;
    
    // Show final statistics
    std::cout << std::endl;
    std::cout << "Final TT Statistics:" << std::endl;
    std::cout << "- Hits: " << engine.tt_table.get_hits() << std::endl;
    std::cout << "- Misses: " << engine.tt_table.get_misses() << std::endl;
    std::cout << "- Writes: " << engine.tt_table.get_writes() << std::endl;
    std::cout << "- Hit Rate: " << std::fixed << std::setprecision(1) 
              << (engine.tt_table.get_hit_rate() * 100.0) << "%" << std::endl;
    
    std::cout << std::endl;
    std::cout << "TT is working! Hits show positions reused, writes show storage." << std::endl;
    
    return 0;
}
