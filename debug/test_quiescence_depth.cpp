// test_quiescence_depth.cpp
// Test to analyze how deep quiescence search goes
#include <iostream>
#include <chrono>
#include "../src/minimal_search.hpp"
#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/movegen_enhanced.hpp"

// Global counter for quiescence depth analysis
int max_quiescence_depth = 0;
int total_quiescence_calls = 0;

int main() {
    std::cout << "=== Quiescence Search Depth Analysis ===" << std::endl;
    std::cout << "Testing how deep quiescence search goes in practice" << std::endl;
    std::cout << "=============================================" << std::endl;

    // Initialize the engine
    Huginn::init();
    
    Huginn::MinimalEngine engine;
    
    // Test Position 1: Many captures possible (tactical melting pot)
    Position pos1;
    pos1.set_from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    
    std::cout << "\nTest Position 1: Tactical Endgame" << std::endl;
    std::cout << "FEN: 8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1" << std::endl;
    
    Huginn::MinimalLimits limits1;
    limits1.max_depth = 3;
    limits1.max_time_ms = 2000;
    
    auto start = std::chrono::steady_clock::now();
    S_MOVE best1 = engine.search(pos1, limits1);
    auto end = std::chrono::steady_clock::now();
    auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Best move: " << engine.move_to_uci(best1) << std::endl;
    std::cout << "Nodes searched: " << engine.nodes_searched << std::endl;
    std::cout << "Time: " << time_ms.count() << "ms" << std::endl;
    
    // Test Position 2: Complex middle game with many pieces
    Position pos2;
    pos2.set_from_fen("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R b KQkq - 0 4");
    
    std::cout << "\nTest Position 2: Italian Game (Complex)" << std::endl;
    std::cout << "FEN: r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R b KQkq - 0 4" << std::endl;
    
    Huginn::MinimalLimits limits2;
    limits2.max_depth = 4;
    limits2.max_time_ms = 3000;
    
    start = std::chrono::steady_clock::now();
    S_MOVE best2 = engine.search(pos2, limits2);
    end = std::chrono::steady_clock::now();
    time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Best move: " << engine.move_to_uci(best2) << std::endl;
    std::cout << "Nodes searched: " << engine.nodes_searched << std::endl;
    std::cout << "Time: " << time_ms.count() << "ms" << std::endl;
    
    // Test Position 3: Very quiet position (should have minimal quiescence)
    Position pos3;
    pos3.set_from_fen("8/8/8/3k4/8/3K4/8/8 w - - 0 1");
    
    std::cout << "\nTest Position 3: Quiet King Endgame" << std::endl;
    std::cout << "FEN: 8/8/8/3k4/8/3K4/8/8 w - - 0 1" << std::endl;
    
    Huginn::MinimalLimits limits3;
    limits3.max_depth = 5;
    limits3.max_time_ms = 1000;
    
    start = std::chrono::steady_clock::now();
    S_MOVE best3 = engine.search(pos3, limits3);
    end = std::chrono::steady_clock::now();
    time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Best move: " << engine.move_to_uci(best3) << std::endl;
    std::cout << "Nodes searched: " << engine.nodes_searched << std::endl;
    std::cout << "Time: " << time_ms.count() << "ms" << std::endl;
    
    // Test capture analysis
    std::cout << "\n--- Capture Generation Analysis ---" << std::endl;
    
    S_MOVELIST all_moves, capture_moves;
    generate_legal_moves_enhanced(pos2, all_moves);
    generate_all_caps(pos2, capture_moves);
    
    std::cout << "Italian Game position:" << std::endl;
    std::cout << "Total legal moves: " << all_moves.count << std::endl;
    std::cout << "Capture moves: " << capture_moves.count << std::endl;
    
    if (capture_moves.count > 0) {
        std::cout << "Available captures: ";
        for (int i = 0; i < capture_moves.count; i++) {
            std::cout << engine.move_to_uci(capture_moves.moves[i]) << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\n=== Quiescence Search Characteristics ===" << std::endl;
    std::cout << "❌ CURRENT ISSUE: No depth limit in quiescence search!" << std::endl;
    std::cout << "⚠️  Risk: Very long capture sequences could cause stack overflow" << std::endl;
    std::cout << "⚠️  Risk: Pathological positions might search extremely deep" << std::endl;
    std::cout << "✅ Benefit: Searches until truly quiet position found" << std::endl;
    std::cout << "✅ Benefit: No artificial cutoff of important tactical sequences" << std::endl;
    
    std::cout << "\n=== Typical Quiescence Depths ===" << std::endl;
    std::cout << "• Quiet positions: 0-1 plies (immediate stand-pat)" << std::endl;
    std::cout << "• Normal positions: 2-4 plies (few captures)" << std::endl;
    std::cout << "• Tactical positions: 4-8 plies (capture sequences)" << std::endl;
    std::cout << "• Extreme positions: 10+ plies (long combinations)" << std::endl;
    
    std::cout << "\n=== Recommendations ===" << std::endl;
    std::cout << "• Add MAX_QUIESCENCE_DEPTH limit (typically 8-12 plies)" << std::endl;
    std::cout << "• Track quiescence ply depth in recursive calls" << std::endl;
    std::cout << "• Consider 'delta pruning' for clearly losing captures" << std::endl;
    std::cout << "• Add quiescence statistics to search info" << std::endl;
    
    return 0;
}
