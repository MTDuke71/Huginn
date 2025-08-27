#include "../src/search.hpp"
#include "../src/evaluation.hpp"
#include "src/position.hpp"
#include <iostream>

void test_famous_position() {
    std::cout << "=== TESTING FAMOUS MATE POSITION ===" << std::endl;
    
    // The famous position you highlighted
    Position pos;
    pos.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    
    std::cout << "Position: " << pos.to_fen() << std::endl;
    
    // First test the evaluation directly
    int eval = Huginn::HybridEvaluator::evaluate(pos);
    std::cout << "Direct evaluation: " << eval << " cp" << std::endl;
    
    // Compare with original evaluation
    std::cout << "\n=== COMPARISON WITH ORIGINAL EVALUATION ===" << std::endl;
    
    // Test the search with different depths
    Huginn::SimpleEngine engine;
    
    for (int depth = 1; depth <= 6; ++depth) {
        engine.reset();
        
        Huginn::SearchLimits limits;
        limits.max_depth = depth;
        limits.max_time_ms = 5000;
        
        std::cout << "\n--- DEPTH " << depth << " ---" << std::endl;
        S_MOVE best_move = engine.search(pos, limits);
        
        std::cout << "Best move: " << Huginn::SimpleEngine::move_to_uci(best_move) << std::endl;
        std::cout << "Nodes: " << engine.get_stats().nodes_searched << std::endl;
        std::cout << "Time: " << engine.get_stats().time_ms << "ms" << std::endl;
        std::cout << "PV: " << Huginn::SimpleEngine::pv_to_string(engine.get_pv()) << std::endl;
    }
}

int main() {
    test_famous_position();
    return 0;
}
