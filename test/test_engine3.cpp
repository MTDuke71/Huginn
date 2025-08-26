#include "Engine3_src/simple_search.hpp"
#include "Engine3_src/hybrid_evaluation.hpp"
#include "src/position.hpp"
#include <iostream>

int main() {
    std::cout << "=== ENGINE3 SIMPLE SEARCH TEST ===" << std::endl;
    
    Engine3::SimpleEngine engine;
    
    // Test 1: Simple mate in 1
    std::cout << "\n### TEST 1: SIMPLE MATE IN 1 ###" << std::endl;
    Position mate_pos;
    mate_pos.set_from_fen("k7/8/1K6/8/8/8/8/7Q w - - 0 1");
    std::cout << "Position: " << mate_pos.to_fen() << std::endl;
    
    Engine3::SearchLimits limits1;
    limits1.max_depth = 3;
    limits1.max_time_ms = 1000;
    
    S_MOVE best_move1 = engine.search(mate_pos, limits1);
    
    std::cout << "Engine3 result:" << std::endl;
    std::cout << "  Best move: " << Engine3::SimpleEngine::move_to_uci(best_move1) << std::endl;
    std::cout << "  Nodes: " << engine.get_stats().nodes_searched << std::endl;
    std::cout << "  Time: " << engine.get_stats().time_ms << "ms" << std::endl;
    std::cout << "  Max depth: " << engine.get_stats().max_depth_reached << std::endl;
    std::cout << "  PV: " << Engine3::SimpleEngine::pv_to_string(engine.get_pv()) << std::endl;
    
    // Test 2: Famous mate position
    std::cout << "\n### TEST 2: FAMOUS MATE POSITION ###" << std::endl;
    Position famous_pos;
    famous_pos.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    std::cout << "Position: " << famous_pos.to_fen() << std::endl;
    
    engine.reset();
    Engine3::SearchLimits limits2;
    limits2.max_depth = 5;
    limits2.max_time_ms = 3000;
    
    S_MOVE best_move2 = engine.search(famous_pos, limits2);
    
    std::cout << "Engine3 result:" << std::endl;
    std::cout << "  Best move: " << Engine3::SimpleEngine::move_to_uci(best_move2) << std::endl;
    std::cout << "  Nodes: " << engine.get_stats().nodes_searched << std::endl;
    std::cout << "  Time: " << engine.get_stats().time_ms << "ms" << std::endl;
    std::cout << "  Max depth: " << engine.get_stats().max_depth_reached << std::endl;
    std::cout << "  PV: " << Engine3::SimpleEngine::pv_to_string(engine.get_pv()) << std::endl;
    
    // Test 3: Back rank mate
    std::cout << "\n### TEST 3: BACK RANK MATE ###" << std::endl;
    Position back_rank;
    back_rank.set_from_fen("r3k2r/8/8/8/8/8/8/4K2R w - - 0 1");
    std::cout << "Position: " << back_rank.to_fen() << std::endl;
    
    engine.reset();
    Engine3::SearchLimits limits3;
    limits3.max_depth = 4;
    limits3.max_time_ms = 2000;
    
    S_MOVE best_move3 = engine.search(back_rank, limits3);
    
    std::cout << "Engine3 result:" << std::endl;
    std::cout << "  Best move: " << Engine3::SimpleEngine::move_to_uci(best_move3) << std::endl;
    std::cout << "  Nodes: " << engine.get_stats().nodes_searched << std::endl;
    std::cout << "  Time: " << engine.get_stats().time_ms << "ms" << std::endl;
    std::cout << "  Max depth: " << engine.get_stats().max_depth_reached << std::endl;
    std::cout << "  PV: " << Engine3::SimpleEngine::pv_to_string(engine.get_pv()) << std::endl;
    
    return 0;
}
