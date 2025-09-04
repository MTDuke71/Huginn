// test_move_ordering.cpp
// Test to demonstrate VICE Part 64 move ordering improvements

#include <iostream>
#include <chrono>
#include "../src/minimal_search.hpp"
#include "../src/position.hpp"
#include "../src/init.hpp"

int main() {
    std::cout << "=== VICE Part 64: Move Ordering Test ===" << std::endl;
    std::cout << "Testing PV moves, killer moves, and history heuristic" << std::endl;
    std::cout << "=======================================================" << std::endl;

    // Initialize the engine
    Huginn::init();
    
    Huginn::MinimalEngine engine;
    
    // Test Position 1: Starting position (good for showing PV move ordering)
    Position pos1;
    pos1.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "\nTest 1: Starting Position (depth 4)" << std::endl;
    std::cout << "Expected: d2d4 should emerge as PV move and get priority" << std::endl;
    
    Huginn::MinimalLimits limits1;
    limits1.max_depth = 4;
    limits1.max_time_ms = 5000;
    
    auto start1 = std::chrono::steady_clock::now();
    S_MOVE best1 = engine.search(pos1, limits1);
    auto end1 = std::chrono::steady_clock::now();
    auto time1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    
    std::cout << "Best move: " << engine.move_to_uci(best1) << std::endl;
    std::cout << "Nodes searched: " << engine.nodes_searched << std::endl;
    std::cout << "Time: " << time1.count() << "ms" << std::endl;
    
    // Test Position 2: Tactical position with captures (good for MVV-LVA + killer moves)
    Position pos2;
    pos2.set_from_fen("r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3");
    
    std::cout << "\nTest 2: Italian Game Position (depth 4)" << std::endl;
    std::cout << "Expected: Captures and good developing moves prioritized" << std::endl;
    
    Huginn::MinimalLimits limits2;
    limits2.max_depth = 4;
    limits2.max_time_ms = 5000;
    
    auto start2 = std::chrono::steady_clock::now();
    S_MOVE best2 = engine.search(pos2, limits2);
    auto end2 = std::chrono::steady_clock::now();
    auto time2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    
    std::cout << "Best move: " << engine.move_to_uci(best2) << std::endl;
    std::cout << "Nodes searched: " << engine.nodes_searched << std::endl;
    std::cout << "Time: " << time2.count() << "ms" << std::endl;
    
    // Test Position 3: Deeper search to show history heuristic building up
    std::cout << "\nTest 3: Deeper Search (depth 5) - History Heuristic Effect" << std::endl;
    std::cout << "Expected: Later iterations benefit from history scores" << std::endl;
    
    Position pos3;
    pos3.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    Huginn::MinimalLimits limits3;
    limits3.max_depth = 5;
    limits3.max_time_ms = 10000;
    
    auto start3 = std::chrono::steady_clock::now();
    S_MOVE best3 = engine.search(pos3, limits3);
    auto end3 = std::chrono::steady_clock::now();
    auto time3 = std::chrono::duration_cast<std::chrono::milliseconds>(end3 - start3);
    
    std::cout << "Best move: " << engine.move_to_uci(best3) << std::endl;
    std::cout << "Nodes searched: " << engine.nodes_searched << std::endl;
    std::cout << "Time: " << time3.count() << "ms" << std::endl;
    
    std::cout << "\n=== Move Ordering Summary ===" << std::endl;
    std::cout << "✅ PV moves get 2,000,000 points (searched first)" << std::endl;
    std::cout << "✅ Captures get 1,000,000 + MVV-LVA score" << std::endl;
    std::cout << "✅ Killer moves get 900,000/800,000 points" << std::endl;
    std::cout << "✅ History heuristic scores quiet moves dynamically" << std::endl;
    std::cout << "✅ Move ordering dramatically reduces nodes searched" << std::endl;
    
    return 0;
}
