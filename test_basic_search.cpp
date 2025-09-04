#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/init.hpp"
#include <iostream>
#include <iomanip>

using namespace Huginn;

int main() {
    init(); // Initialize engine subsystems
    
    std::cout << "=== VICE Part 60: Basic Search Testing ===\n";
    std::cout << "Testing engine with tactical positions and move ordering analysis\n\n";
    
    MinimalEngine engine;
    Position pos;
    SearchInfo info;
    
    // Test 1: WAC1 - Mate in 3 position (4:04, 5:16)
    std::cout << "TEST 1: WAC1 - Mate in 3 Position\n";
    std::cout << "==================================\n";
    std::cout << "FEN: 2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1\n";
    std::cout << "Expected: Qg6+ or similar forcing move leading to mate\n\n";
    
    pos.set_from_fen("2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1");
    
    // Search to depth 4 as mentioned in video (2:00)
    info = SearchInfo();
    info.max_depth = 4;
    info.stopped = false;
    info.fh = 0;
    info.fhf = 0;
    
    std::cout << "Searching to depth 4...\n";
    S_MOVE best_move = engine.searchPosition(pos, info);
    
    std::cout << "\\nSearch Results:\n";
    std::cout << "Best move: " << engine.move_to_uci(best_move) << std::endl;
    // Note: searchPosition doesn't return score directly, would need to get from search
    std::cout << "Nodes searched: " << info.nodes << std::endl;
    
    // VICE Part 60: Move ordering analysis (0:13)
    std::cout << "\\nMove Ordering Statistics:\n";
    std::cout << "Fail highs (fh): " << info.fh << std::endl;
    std::cout << "Fail high first (fhf): " << info.fhf << std::endl;
    if (info.fh > 0) {
        double ordering_ratio = (double)info.fhf / (double)info.fh * 100.0;
        std::cout << "Move ordering efficiency: " << std::fixed << std::setprecision(1) 
                  << ordering_ratio << "% (higher is better)\\n";
        if (ordering_ratio > 90) {
            std::cout << "Excellent move ordering!\\n";
        } else if (ordering_ratio > 70) {
            std::cout << "Good move ordering.\\n";
        } else {
            std::cout << "Move ordering needs improvement.\\n";
        }
    }
    
    // Test 2: Starting position at depth 4 (2:00)
    std::cout << "\\n\\nTEST 2: Starting Position at Depth 4\n";
    std::cout << "=====================================\n";
    std::cout << "Testing with 'massive horizon effects' as mentioned in video\\n\\n";
    
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    info = SearchInfo();
    info.max_depth = 4;
    info.stopped = false;
    info.fh = 0;
    info.fhf = 0;
    
    S_MOVE starting_best = engine.searchPosition(pos, info);
    
    std::cout << "Search Results:\\n";
    std::cout << "Best move: " << engine.move_to_uci(starting_best) << std::endl;
    std::cout << "Nodes searched: " << info.nodes << std::endl;
    
    std::cout << "\\nMove Ordering Statistics:\\n";
    std::cout << "Fail highs (fh): " << info.fh << std::endl;
    std::cout << "Fail high first (fhf): " << info.fhf << std::endl;
    if (info.fh > 0) {
        double ordering_ratio = (double)info.fhf / (double)info.fh * 100.0;
        std::cout << "Move ordering efficiency: " << std::fixed << std::setprecision(1) 
                  << ordering_ratio << "%\\n";
    }
    
    std::cout << "\\n=== VICE Part 60 Testing Complete ===\\n";
    std::cout << "Next steps: Improve move ordering and implement quiescence search (5:57)\\n";
    
    return 0;
}
