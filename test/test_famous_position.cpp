#include "../src/minimal_search.hpp"
#include "../src/evaluation.hpp"
#include "../src/init.hpp"
#include "src/position.hpp"
#include <iostream>

void test_famous_position() {
    std::cout << "=== TESTING FAMOUS MATE POSITION ===" << std::endl;
    
    // The famous position you highlighted
    Position pos;
    pos.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    
    std::cout << "Position: " << pos.to_fen() << std::endl;
    
    // First test the evaluation directly using stable MinimalEngine
    Huginn::MinimalEngine engine;
    int eval = engine.evalPosition(pos);
    std::cout << "Direct evaluation: " << eval << " cp" << std::endl;
    
    // Compare with original evaluation
    std::cout << "\n=== COMPARISON WITH ORIGINAL EVALUATION ===" << std::endl;
    
    // Test the search with different depths
    for (int depth = 1; depth <= 6; ++depth) {
        
        Huginn::MinimalLimits limits;
        limits.max_depth = depth;
        limits.max_time_ms = 5000;
        
        std::cout << "\n--- DEPTH " << depth << " ---" << std::endl;
        S_MOVE best_move = engine.search(pos, limits);
        
        std::cout << "Best move: " << engine.move_to_uci(best_move) << std::endl;
        std::cout << "Nodes: " << engine.nodes_searched << std::endl;
        // Note: MinimalEngine doesn't expose detailed stats like the old SimpleEngine
    }
}

int main() {
    Huginn::init();
    test_famous_position();
    return 0;
}
