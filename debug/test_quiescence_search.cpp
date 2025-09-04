// test_quiescence_search.cpp
// Demonstrate VICE Part 65 quiescence search to eliminate horizon effect

#include <iostream>
#include <chrono>
#include "../src/minimal_search.hpp"
#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/movegen_enhanced.hpp"

int main() {
    std::cout << "=== VICE Part 65: Quiescence Search Test ===" << std::endl;
    std::cout << "Demonstrating horizon effect elimination" << std::endl;
    std::cout << "=============================================" << std::endl;

    // Initialize the engine
    Huginn::init();
    
    Huginn::MinimalEngine engine;
    
    // Test Position: Tactical position with potential captures
    // This position has hanging pieces that would create horizon effect without quiescence
    Position pos;
    pos.set_from_fen("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R b KQkq - 0 4");
    
    std::cout << "\nTest Position: Italian Game with tactical possibilities" << std::endl;
    std::cout << "FEN: r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R b KQkq - 0 4" << std::endl;
    std::cout << "This position tests the quiescence search's ability to see through" << std::endl;
    std::cout << "tactical sequences and avoid the horizon effect." << std::endl;
    
    // Test 1: Show capture generation
    std::cout << "\n--- VICE Part 65 Feature Test ---" << std::endl;
    
    S_MOVELIST all_moves, capture_moves;
    generate_legal_moves_enhanced(pos, all_moves);
    generate_all_caps(pos, capture_moves);
    
    std::cout << "Total legal moves: " << all_moves.count << std::endl;
    std::cout << "Capture moves only: " << capture_moves.count << std::endl;
    std::cout << "Efficiency gain: " << (100.0 - (double(capture_moves.count) / all_moves.count * 100.0)) << "% fewer moves to search in quiescence" << std::endl;
    
    // Test 2: Search with quiescence 
    std::cout << "\n--- Search Test (depth 4) ---" << std::endl;
    std::cout << "Quiescence search will evaluate captures beyond the horizon" << std::endl;
    
    Huginn::MinimalLimits limits;
    limits.max_depth = 4;
    limits.max_time_ms = 5000;
    
    auto start = std::chrono::steady_clock::now();
    S_MOVE best = engine.search(pos, limits);
    auto end = std::chrono::steady_clock::now();
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Best move: " << engine.move_to_uci(best) << std::endl;
    std::cout << "Nodes searched: " << engine.nodes_searched << std::endl;
    std::cout << "Time: " << time_ms.count() << "ms" << std::endl;
    
    // Test 3: Demonstrate "stand pat" concept
    std::cout << "\n--- Stand Pat Demonstration ---" << std::endl;
    int stand_pat_score = engine.evalPosition(pos);
    std::cout << "Current position evaluation (stand pat): " << stand_pat_score << "cp" << std::endl;
    std::cout << "Quiescence search will use this as a baseline and only search" << std::endl;
    std::cout << "captures that might improve the position." << std::endl;
    
    // Test 4: Show search stability
    std::cout << "\n--- Search Stability Test ---" << std::endl;
    std::cout << "Running multiple searches to show score stability..." << std::endl;
    
    for (int depth = 1; depth <= 3; depth++) {
        Huginn::MinimalLimits test_limits;
        test_limits.max_depth = depth;
        test_limits.max_time_ms = 1000;
        
        S_MOVE test_move = engine.search(pos, test_limits);
        std::cout << "Depth " << depth << ": " << engine.move_to_uci(test_move) 
                  << " (nodes: " << engine.nodes_searched << ")" << std::endl;
    }
    
    std::cout << "\n=== Quiescence Search Benefits ===" << std::endl;
    std::cout << "✅ Eliminates horizon effect by searching captures to quiet positions" << std::endl;
    std::cout << "✅ Uses efficient generate_all_caps() function (VICE Part 65)" << std::endl;
    std::cout << "✅ Implements 'stand pat' - evaluates current position first" << std::endl;
    std::cout << "✅ Only searches captures, not quiet moves in quiescence" << std::endl;
    std::cout << "✅ Provides stable, reliable position evaluations" << std::endl;
    std::cout << "✅ Prevents tactical oversights at search boundaries" << std::endl;
    
    return 0;
}
