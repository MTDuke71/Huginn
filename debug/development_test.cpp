#include "../src/minimal_search.hpp"
#include "../src/position.hpp"
#include "../src/evaluation.hpp"
#include "../src/init.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    init();
    
    std::cout << "=== DEVELOPMENT vs PAWN MOVE TEST ===" << std::endl;
    
    MinimalEngine engine;
    Position pos;
    SearchInfo info;
    
    // Test position after 1.c4 - what should Black do?
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 1");
    
    info = SearchInfo();
    info.max_depth = 10;  // Increased from 6 to 10
    info.stopped = false;
    info.fh = 0;
    info.fhf = 0;
    
    std::cout << "Position after 1.c4 - Black to move:" << std::endl;
    std::cout << "Testing at depth 10 to ensure evaluation stability..." << std::endl;
    
    S_MOVE best_move = engine.searchPosition(pos, info);
    std::cout << "Best move found: " << engine.move_to_uci(best_move) << std::endl;
    std::cout << "Nodes searched: " << info.nodes << std::endl;
    std::cout << "Expected: developing move (Nf6, Nc6, e5, d5)" << std::endl;
    std::cout << "NOT: random pawn move like b6, a6, h6" << std::endl;
    
    return 0;
}
