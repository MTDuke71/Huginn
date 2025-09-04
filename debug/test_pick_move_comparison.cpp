#include "../src/minimal_search.hpp"
#include "../src/position.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace Huginn;

int main() {
    std::cout << "=== VICE Part 62: Move Ordering Comparison Test ===\n\n";
    
    init();
    
    // Load WAC.1 tactical position
    Position pos;
    std::string test_fen = "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3QP/PPB4P/R4RK1 w - - 0 1";
    if (!pos.set_from_fen(test_fen)) {
        std::cout << "Failed to load test position\n";
        return 1;
    }
    
    std::cout << "Position: " << test_fen << std::endl;
    std::cout << "WAC.1 - Mate in 3 tactical position\n\n";
    
    MinimalEngine engine;
    
    // Test with pick_next_move (our current implementation)
    std::cout << "=== Testing with VICE Part 62 'Pick Next Move' ===\n";
    
    SearchInfo info1;
    info1.max_depth = 5;
    info1.depth_only = true;
    info1.nodes = 0;
    info1.fh = 0;
    info1.fhf = 0;
    
    auto start1 = std::chrono::steady_clock::now();
    S_MOVE best_move1 = engine.searchPosition(pos, info1);
    auto end1 = std::chrono::steady_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    
    std::cout << "Results with Pick Next Move:\n";
    std::cout << "Best move: " << engine.move_to_uci(best_move1) << std::endl;
    std::cout << "Nodes searched: " << info1.nodes << std::endl;
    std::cout << "Time taken: " << duration1.count() << " ms\n";
    std::cout << "Total fail highs: " << info1.fh << std::endl;
    std::cout << "Fail high first: " << info1.fhf << std::endl;
    
    double ordering1 = 0.0;
    if (info1.fh > 0) {
        ordering1 = (double(info1.fhf) / double(info1.fh)) * 100.0;
        std::cout << "Move ordering: " << std::fixed << std::setprecision(1) << ordering1 << "%\n";
    }
    
    uint64_t nps1 = 0;
    if (duration1.count() > 0) {
        nps1 = (info1.nodes * 1000) / duration1.count();
        std::cout << "Nodes per second: " << nps1 << std::endl;
    }
    
    std::cout << "\n=== VICE Part 62 Analysis ===\n";
    std::cout << "Expected from video:\n";
    std::cout << "- Move ordering improvement: From ~10% to ~96%\n";
    std::cout << "- Node reduction: From ~8.5M to ~151K nodes\n";
    std::cout << "- Dramatic performance improvement\n\n";
    
    std::cout << "Our implementation results:\n";
    std::cout << "- Move ordering achieved: " << ordering1 << "%\n";
    std::cout << "- Nodes searched: " << info1.nodes << std::endl;
    std::cout << "- Performance: " << nps1 << " nps\n\n";
    
    if (ordering1 > 95.0) {
        std::cout << "✅ EXCELLENT: Move ordering exceeds video expectations!\n";
    } else if (ordering1 > 80.0) {
        std::cout << "✅ VERY GOOD: High move ordering achieved.\n";
    } else if (ordering1 > 50.0) {
        std::cout << "✅ GOOD: Decent move ordering improvement.\n";
    } else {
        std::cout << "⚠️  NEEDS WORK: Move ordering could be better.\n";
    }
    
    if (info1.nodes < 500000) {
        std::cout << "✅ EFFICIENT: Low node count for depth 5.\n";
    } else if (info1.nodes < 2000000) {
        std::cout << "✅ REASONABLE: Acceptable node count.\n";
    } else {
        std::cout << "⚠️  HIGH NODES: Could be more efficient.\n";
    }
    
    std::cout << "\n=== Key Improvements from VICE Part 62 ===\n";
    std::cout << "1. Pick Next Move: Dynamically select best remaining move\n";
    std::cout << "2. MVV-LVA Scoring: Prioritize valuable captures\n";
    std::cout << "3. Incremental Selection: More efficient than full sorting\n";
    std::cout << "4. Better Beta Cutoffs: First move more likely to cause cutoff\n";
    
    std::cout << "\n=== Test Complete ===\n";
    return 0;
}
