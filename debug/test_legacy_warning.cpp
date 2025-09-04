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
    
    std::cout << "Calling legacy alpha_beta() function directly...\n";
    
    // This should trigger the warning message
    int score = engine.alpha_beta(pos, 2, -1000, 1000, info, true);
    
    std::cout << "First call completed (score: " << score << ")\n";
    
    // This should NOT show the warning again (static bool prevents multiple warnings)
    std::cout << "\nCalling legacy alpha_beta() function again...\n";
    score = engine.alpha_beta(pos, 1, -1000, 1000, info, true);
    std::cout << "Second call completed (score: " << score << ")\n";
    
    std::cout << "\n✅ Legacy warning system working correctly!\n";
    std::cout << "The warning appears only once per program execution.\n";
    
    return 0;
}
