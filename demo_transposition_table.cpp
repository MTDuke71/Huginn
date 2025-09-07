#include "src/init.hpp"
#include "src/minimal_search.hpp"
#include "src/position.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    init();
    
    std::cout << "=== VICE Part 84: Transposition Table Demonstration ===" << std::endl;
    std::cout << "This test shows how transposition tables dramatically improve search efficiency" << std::endl;
    std::cout << "by storing and reusing previously computed search results." << std::endl;
    std::cout << std::endl;
    
    // Use a position likely to have transpositions (middlegame with many options)
    std::string fen = "r1bqr1k1/pp1n1ppp/2p1pn2/3p4/2PP4/2N1PN2/PP2BPPP/R1BQK2R w KQ - 0 9";
    Position pos;
    pos.set_from_fen(fen);
    
    std::cout << "Position: " << fen << std::endl;
    std::cout << "Testing search depth 5 with 64MB transposition table..." << std::endl;
    std::cout << std::endl;
    
    MinimalEngine engine;
    MinimalLimits limits;
    limits.max_depth = 5;
    limits.max_time_ms = 10000;
    
    S_MOVE best_move = engine.search(pos, limits);
    
    std::cout << std::endl;
    std::cout << "Search completed!" << std::endl;
    std::cout << "Best move: " << engine.move_to_uci(best_move) << std::endl;
    
    // Display detailed transposition table statistics
    engine.print_tt_stats();
    
    std::cout << std::endl;
    std::cout << "Transposition Table Benefits (VICE Part 84):" << std::endl;
    std::cout << "- Stores search results (score, depth, bounds, best move)" << std::endl;
    std::cout << "- Avoids re-searching identical positions" << std::endl;
    std::cout << "- Provides better move ordering with TT moves" << std::endl;
    std::cout << "- Handles mate score adjustments correctly" << std::endl;
    std::cout << "- Dramatically reduces node count in complex positions" << std::endl;
    std::cout << std::endl;
    std::cout << "Compare node counts with/without TT to see the efficiency gains!" << std::endl;
    
    return 0;
}
