#include "../src/minimal_search.hpp"
#include "../src/board.hpp"
#include "../src/position.hpp"
#include "../src/move.hpp"
#include "../src/movegen_enhanced.hpp"
#include "../src/chess_types.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace Huginn;

int main() {
    std::cout << "=== VICE Part 62: Move Ordering - Picking a Move Test ===\n\n";
    
    // Initialize the chess board and position
    init();  // Initialize engine subsystems
    
    // Load a tactical position for testing
    Position pos;
    std::string test_fen = "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3QP/PPB4P/R4RK1 w - - 0 1";  // WAC.1
    if (!pos.set_from_fen(test_fen)) {
        std::cout << "Failed to load WAC.1 position, trying starting position\n";
        
        // Fallback to starting position
        test_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        if (!pos.set_from_fen(test_fen)) {
            std::cout << "Failed to load starting position\n";
            return 1;
        }
    }
    
    MinimalEngine engine;
    
    std::cout << "Position: " << test_fen << "\n";
    if (test_fen.find("WAC") != std::string::npos || test_fen.find("2rr3k") != std::string::npos) {
        std::cout << "This is WAC.1 - a mate in 3 position\n";
    } else {
        std::cout << "Testing with starting position\n";
    }
    std::cout << std::endl;
    
    // Configure search for move ordering analysis
    SearchInfo info;
    info.max_depth = 5;  // Reduce depth for faster test
    info.depth_only = true;  // Bypass time management
    info.nodes = 0;
    info.fh = 0;   // Fail high count
    info.fhf = 0;  // Fail high first
    
    auto start_time = std::chrono::steady_clock::now();
    
    // Search the position
    S_MOVE best_move = engine.searchPosition(pos, info);
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Search Results:\n";
    std::cout << "===============\n";
    std::cout << "Best move: " << engine.move_to_uci(best_move) << std::endl;
    std::cout << "Depth searched: " << info.max_depth << std::endl;
    std::cout << "Nodes searched: " << info.nodes << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms\n";
    
    // Calculate nodes per second
    if (duration.count() > 0) {
        uint64_t nps = (info.nodes * 1000) / duration.count();
        std::cout << "Nodes per second: " << nps << std::endl;
    }
    
    // VICE Part 62: Move Ordering Statistics (Key improvement from video)
    std::cout << "\nMove Ordering Statistics:\n";
    std::cout << "========================\n";
    std::cout << "Total fail highs: " << info.fh << std::endl;
    std::cout << "Fail high first: " << info.fhf << std::endl;
    
    if (info.fh > 0) {
        double ordering_percentage = (double(info.fhf) / double(info.fh)) * 100.0;
        std::cout << "Move ordering percentage: " << std::fixed << std::setprecision(1) 
                  << ordering_percentage << "%\n";
        
        std::cout << "\nVICE Part 62 Improvement:\n";
        std::cout << "========================\n";
        std::cout << "Expected improvement: From ~10% to ~96% move ordering\n";
        std::cout << "Expected node reduction: From ~8.5M to ~151K nodes\n";
        std::cout << "Current result: " << ordering_percentage << "% ordering with " 
                  << info.nodes << " nodes\n";
        
        if (ordering_percentage > 80.0) {
            std::cout << "✅ EXCELLENT: High move ordering percentage achieved!\n";
        } else if (ordering_percentage > 50.0) {
            std::cout << "✅ GOOD: Decent move ordering percentage.\n";
        } else {
            std::cout << "⚠️  NEEDS IMPROVEMENT: Low move ordering percentage.\n";
        }
    }
    
    std::cout << "\n=== Test Complete ===\n";
    return 0;
}
