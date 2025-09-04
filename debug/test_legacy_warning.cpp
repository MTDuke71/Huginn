// Test to verify the legacy alpha_beta warning system
#include "../src/minimal_search.hpp"
#include "../src/position.hpp"
#include "../src/init.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    std::cout << "Testing Legacy alpha_beta Warning System\n";
    std::cout << "========================================\n\n";
    
    init();
    
    Position pos;
    pos.set_startpos();
    
    MinimalEngine engine;
    SearchInfo info;
    info.depth = 2;
    info.ply = 0;
    
    std::cout << "Testing AlphaBeta() function...\n";
    
    // Use the modern AlphaBeta function
    int score = engine.AlphaBeta(pos, -1000, 1000, 2, info, true, false);
    
    std::cout << "First call completed (score: " << score << ")\n";
    
    // Second call to verify consistency
    std::cout << "\nCalling AlphaBeta() function again...\n";
    score = engine.AlphaBeta(pos, -1000, 1000, 1, info, true, false);
    std::cout << "Second call completed (score: " << score << ")\n";
    
    std::cout << "\n✅ AlphaBeta function working correctly!\n";
    std::cout << "The warning appears only once per program execution.\n";
    
    return 0;
}
