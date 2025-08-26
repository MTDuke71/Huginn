#include "src/position.hpp"
#include "src/evaluation.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/search.hpp"
#include "src/attack_detection.hpp"
#include <iostream>

int main() {
    std::cout << "=== REAL SEARCH ENGINE TEST ===" << std::endl;
    
    Search::Engine search_engine;
    search_engine.set_threads(1); // Single thread for deterministic results
    
    // Test 1: Simple mate in 1
    std::cout << "\n### TEST 1: SIMPLE MATE IN 1 ###" << std::endl;
    Position simple_mate;
    simple_mate.set_from_fen("k7/8/1K6/8/8/8/8/7Q w - - 0 1");
    std::cout << "Position: " << simple_mate.to_fen() << std::endl;
    
    // Create search limits
    Search::SearchLimits limits1;
    limits1.max_depth = 1;
    limits1.threads = 1;
    
    S_MOVE best_move1 = search_engine.search(simple_mate, limits1);
    std::cout << "Depth 1 search result:" << std::endl;
    std::cout << "  Best move: " << Search::move_to_uci(best_move1) << std::endl;
    std::cout << "  Stats: " << search_engine.get_stats().nodes_searched << " nodes" << std::endl;
    
    // Test deeper search
    Search::SearchLimits limits1_deep;
    limits1_deep.max_depth = 3;
    limits1_deep.threads = 1;
    
    S_MOVE best_move1_deep = search_engine.search(simple_mate, limits1_deep);
    std::cout << "Depth 3 search result:" << std::endl;
    std::cout << "  Best move: " << Search::move_to_uci(best_move1_deep) << std::endl;
    std::cout << "  Stats: " << search_engine.get_stats().nodes_searched << " nodes" << std::endl;
    
    // Print PV
    const Search::PVLine& pv = search_engine.get_pv();
    std::cout << "  PV: ";
    for (int i = 0; i < pv.length; ++i) {
        std::cout << Search::move_to_uci(pv.moves[i]) << " ";
    }
    std::cout << std::endl;
    
    // Test 2: Back rank mate
    std::cout << "\n### TEST 2: BACK RANK MATE ###" << std::endl;
    Position back_rank;
    back_rank.set_from_fen("6k1/5ppp/8/8/8/8/5PPP/4R1K1 w - - 0 1");
    std::cout << "Position: " << back_rank.to_fen() << std::endl;
    
    Search::SearchLimits limits2;
    limits2.max_depth = 1;
    limits2.threads = 1;
    
    S_MOVE best_move2 = search_engine.search(back_rank, limits2);
    std::cout << "Depth 1 search result:" << std::endl;
    std::cout << "  Best move: " << Search::move_to_uci(best_move2) << std::endl;
    
    // Test deeper search
    Search::SearchLimits limits2_deep;
    limits2_deep.max_depth = 3;
    limits2_deep.threads = 1;
    
    S_MOVE best_move2_deep = search_engine.search(back_rank, limits2_deep);
    std::cout << "Depth 3 search result:" << std::endl;
    std::cout << "  Best move: " << Search::move_to_uci(best_move2_deep) << std::endl;
    
    // Print PV
    const Search::PVLine& pv2 = search_engine.get_pv();
    std::cout << "  PV: ";
    for (int i = 0; i < pv2.length; ++i) {
        std::cout << Search::move_to_uci(pv2.moves[i]) << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
