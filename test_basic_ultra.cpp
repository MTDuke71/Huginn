#include <iostream>
#include "ultra_engine/ultra_move.hpp"

using namespace UltraEngine;

int main() {
    std::cout << "Testing basic UltraMove functionality...\n";
    
    try {
        UltraMoveList moves;
        std::cout << "✓ UltraMoveList created successfully\n";
        
        moves.add_quiet(0, 8);
        std::cout << "✓ Added quiet move\n";
        
        moves.add_capture(8, 16, 5);
        std::cout << "✓ Added capture move\n";
        
        std::cout << "Total moves: " << moves.size() << "\n";
        
        std::cout << "✅ Basic UltraMove test passed!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown exception!" << std::endl;
        return 1;
    }
    
    return 0;
}