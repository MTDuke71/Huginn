#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/minimal_search.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    std::cout << "=== VICE Part 60: Basic Search Testing ===" << std::endl;
    std::cout << "Demonstrating move ordering statistics with fail high analysis" << std::endl;
    
    // Initialize Huginn system
    init();
    std::cout << "Huginn system initialized" << std::endl;

    Position pos;
    MinimalEngine engine;
    
    // Test 1: Starting position
    std::cout << "\n=== Test 1: Starting Position Analysis ===" << std::endl;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        std::cout << "Failed to set starting position" << std::endl;
        return 1;
    }
    
    SearchInfo info;
    info.max_depth = 4;
    auto start_time = std::chrono::steady_clock::now();
    info.start_time = start_time;
    info.stop_time = start_time + std::chrono::milliseconds(2000);
    
    std::cout << "Searching starting position to depth " << info.max_depth << std::endl;
    S_MOVE best_move = engine.searchPosition(pos, info);
    
    // Display VICE Part 60 style statistics
    std::cout << "\n=== VICE Part 60 Move Ordering Statistics ===" << std::endl;
    std::cout << "Total nodes searched: " << info.nodes << std::endl;
    std::cout << "Fail highs (fh): " << info.fh << std::endl;
    std::cout << "Fail high first (fhf): " << info.fhf << std::endl;
    if (info.fh > 0) {
        double ordering_ratio = (double)info.fhf / info.fh * 100.0;
        std::cout << "Move ordering efficiency: " << ordering_ratio << "%" << std::endl;
    }
    
    // Test 2: WAC.1 - Mate in 3 (from image: 2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - -)
    std::cout << "\n=== Test 2: WAC.1 Tactical Position (Mate in 3) ===" << std::endl;
    if (!pos.set_from_fen("2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1")) {
        std::cout << "Failed to set WAC.1 position" << std::endl;
        return 1;
    }
    
    SearchInfo info2;
    info2.max_depth = 6;  // Should find mate in 3
    auto start_time2 = std::chrono::steady_clock::now();
    info2.start_time = start_time2;
    info2.stop_time = start_time2 + std::chrono::milliseconds(15000);  // 15 seconds for complex tactical position
    
    std::cout << "Searching WAC.1 (mate in 3) to depth " << info2.max_depth << std::endl;
    best_move = engine.searchPosition(pos, info2);
    
    std::cout << "\n=== VICE Part 60 Move Ordering Statistics (WAC.1) ===" << std::endl;
    std::cout << "Total nodes searched: " << info2.nodes << std::endl;
    std::cout << "Fail highs (fh): " << info2.fh << std::endl;
    std::cout << "Fail high first (fhf): " << info2.fhf << std::endl;
    if (info2.fh > 0) {
        double ordering_ratio = (double)info2.fhf / info2.fh * 100.0;
        std::cout << "Move ordering efficiency: " << ordering_ratio << "%" << std::endl;
    }
    
    std::cout << "\n=== Search Performance Summary ===" << std::endl;
    std::cout << "Engine successfully completed VICE Part 60 Basic Search Testing" << std::endl;
    std::cout << "✓ Move ordering statistics implemented" << std::endl;
    std::cout << "✓ Alpha-beta search with fail high tracking" << std::endl;
    std::cout << "✓ Tactical position analysis" << std::endl;
    
    return 0;
}
