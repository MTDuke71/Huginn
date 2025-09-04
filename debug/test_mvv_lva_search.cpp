// Test MVV-LVA impact on search efficiency
// Compare search performance with and without MVV-LVA move ordering

#include "../src/minimal_search.hpp"
#include "../src/position.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <chrono>

using namespace Huginn;

void test_search_efficiency() {
    std::cout << "=== MVV-LVA Search Efficiency Test ===\n";
    
    init();
    
    // Test on a tactical position with many captures
    Position pos;
    std::string tactical_fen = "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3QP/PPB4P/R4RK1 w - - 0 1";
    
    if (!pos.set_from_fen(tactical_fen)) {
        std::cout << "Failed to parse tactical FEN!\n";
        return;
    }
    
    std::cout << "Testing position: " << tactical_fen << "\n";
    std::cout << "This is WAC.1 - a mate in 3 position with many captures\n\n";
    
    MinimalEngine engine;
    
    // Use SearchInfo for depth-only search (bypasses time management)
    SearchInfo info;
    info.depth_only = true;  // Bypass time management
    info.max_depth = 6;      // Search to depth 6
    info.infinite = true;    // Don't stop due to time
    
    std::cout << "Running depth-only search to depth " << info.max_depth << "...\n";
    
    auto start = std::chrono::steady_clock::now();
    S_MOVE best_move = engine.searchPosition(pos, info);
    auto end = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Search completed!\n";
    std::cout << "Time taken: " << duration.count() << " ms\n";
    std::cout << "Nodes searched: " << info.nodes << "\n";
    
    if (best_move.move != 0) {
        // Decode the best move
        int from = best_move.get_from();
        int to = best_move.get_to();
        
        char from_file = 'a' + ((from % 10) - 1);
        char from_rank = '1' + ((from / 10) - 2);
        char to_file = 'a' + ((to % 10) - 1);
        char to_rank = '1' + ((to / 10) - 2);
        
        std::cout << "Best move: " << from_file << from_rank << to_file << to_rank;
        
        if (best_move.is_capture()) {
            std::cout << " (capture)";
        }
        if (best_move.is_promotion()) {
            std::cout << " (promotion)";
        }
        std::cout << "\n";
    } else {
        std::cout << "No move found!\n";
    }
    
    // Calculate nodes per second
    if (duration.count() > 0) {
        double nps = (double)info.nodes / (duration.count() / 1000.0);
        std::cout << "Nodes per second: " << (int)nps << "\n";
    }
    
    std::cout << "\nWith MVV-LVA move ordering, the engine should:\n";
    std::cout << "1. Search promising captures first\n";
    std::cout << "2. Find alpha-beta cutoffs faster\n";
    std::cout << "3. Achieve better search efficiency\n";
}

void test_move_ordering_comparison() {
    std::cout << "\n=== Move Ordering Statistics ===\n";
    
    // Test a position where MVV-LVA should help significantly
    Position pos;
    pos.set_startpos();  // Start from initial position
    
    MinimalEngine engine;
    
    // Generate moves and show ordering
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    
    std::cout << "Starting position - " << move_list.count << " legal moves\n";
    std::cout << "Before MVV-LVA ordering (first 10):\n";
    
    for (int i = 0; i < std::min(10, move_list.count); i++) {
        S_MOVE move = move_list.moves[i];
        int from = move.get_from();
        int to = move.get_to();
        
        char from_file = 'a' + ((from % 10) - 1);
        char from_rank = '1' + ((from / 10) - 2);
        char to_file = 'a' + ((to % 10) - 1);
        char to_rank = '1' + ((to / 10) - 2);
        
        std::cout << (i+1) << ". " << from_file << from_rank << to_file << to_rank 
                  << " (score: " << move.score << ")\n";
    }
    
    // Apply MVV-LVA ordering
    engine.order_moves(move_list, pos);
    
    std::cout << "\nAfter MVV-LVA ordering (first 10):\n";
    for (int i = 0; i < std::min(10, move_list.count); i++) {
        S_MOVE move = move_list.moves[i];
        int from = move.get_from();
        int to = move.get_to();
        
        char from_file = 'a' + ((from % 10) - 1);
        char from_rank = '1' + ((from / 10) - 2);
        char to_file = 'a' + ((to % 10) - 1);
        char to_rank = '1' + ((to / 10) - 2);
        
        std::cout << (i+1) << ". " << from_file << from_rank << to_file << to_rank 
                  << " (score: " << move.score << ")";
        
        if (move.is_capture()) {
            std::cout << " [CAPTURE]";
        } else if (move.is_promotion()) {
            std::cout << " [PROMOTION]";
        } else {
            std::cout << " [QUIET]";
        }
        std::cout << "\n";
    }
}

int main() {
    std::cout << "VICE Tutorial: MVV-LVA Search Efficiency Testing\n";
    std::cout << "===============================================\n\n";
    
    try {
        test_move_ordering_comparison();
        test_search_efficiency();
        
        std::cout << "\n=== MVV-LVA Search Testing Complete ===\n";
        std::cout << "MVV-LVA move ordering is now integrated into the search!\n";
        std::cout << "This should significantly improve alpha-beta performance\n";
        std::cout << "by examining the most promising captures first.\n";
        
    } catch (const std::exception& e) {
        std::cout << "Error during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
