#include "src/minimal_search.hpp"
#include "src/position.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    std::cout << "=== VICE Part 57 - Clear To Search Test ===" << std::endl;
    
    // Create engine and position
    MinimalEngine engine;
    Position pos;
    pos.set_startpos();
    
    // Create SearchInfo
    SearchInfo info;
    info.max_depth = 5;
    info.infinite = false;
    
    // Simulate some search state (dirty state)
    std::cout << "Before clearForSearch:" << std::endl;
    std::cout << "  info.ply = " << info.ply << std::endl;
    std::cout << "  info.nodes = " << info.nodes << std::endl;
    std::cout << "  info.stopped = " << (info.stopped ? "true" : "false") << std::endl;
    std::cout << "  engine.nodes_searched = " << engine.nodes_searched << std::endl;
    std::cout << "  engine.should_stop = " << (engine.should_stop ? "true" : "false") << std::endl;
    
    // Dirty the state
    info.ply = 10;
    info.nodes = 5000;
    info.stopped = true;
    engine.nodes_searched = 3000;
    engine.should_stop = true;
    
    // Set some search history (simulate previous search)
    engine.search_history[1][25] = 100;  // Some arbitrary history score
    
    std::cout << "\nAfter dirtying state:" << std::endl;
    std::cout << "  info.ply = " << info.ply << std::endl;
    std::cout << "  info.nodes = " << info.nodes << std::endl;
    std::cout << "  info.stopped = " << (info.stopped ? "true" : "false") << std::endl;
    std::cout << "  engine.nodes_searched = " << engine.nodes_searched << std::endl;
    std::cout << "  engine.should_stop = " << (engine.should_stop ? "true" : "false") << std::endl;
    std::cout << "  engine.search_history[1][25] = " << engine.search_history[1][25] << std::endl;
    
    // Test VICE Part 57 - clearForSearch function
    std::cout << "\n>>> Calling clearForSearch (VICE Part 57) <<<" << std::endl;
    MinimalEngine::clearForSearch(engine, info);
    
    std::cout << "\nAfter clearForSearch:" << std::endl;
    std::cout << "  info.ply = " << info.ply << " (should be 0)" << std::endl;
    std::cout << "  info.nodes = " << info.nodes << " (should be 0)" << std::endl;
    std::cout << "  info.stopped = " << (info.stopped ? "true" : "false") << " (should be false)" << std::endl;
    std::cout << "  info.quit = " << (info.quit ? "true" : "false") << " (should be false)" << std::endl;
    std::cout << "  engine.nodes_searched = " << engine.nodes_searched << " (should be 0)" << std::endl;
    std::cout << "  engine.should_stop = " << (engine.should_stop ? "true" : "false") << " (should be false)" << std::endl;
    std::cout << "  engine.search_history[1][25] = " << engine.search_history[1][25] << " (should be 0)" << std::endl;
    
    // Test the VICE-style search function
    std::cout << "\n=== Testing VICE-style searchPosition function ===" << std::endl;
    info.max_depth = 3;  // Shallow search for demonstration
    
    S_MOVE best_move = engine.searchPosition(pos, info);
    
    std::cout << "\nSearch completed!" << std::endl;
    std::cout << "Best move: " << engine.move_to_uci(best_move) << std::endl;
    std::cout << "Nodes searched: " << info.nodes << std::endl;
    
    return 0;
}
