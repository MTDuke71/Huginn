#include <iostream>
#include <intrin.h>
#include "ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

int main() {
    std::cout << "Testing UltraAttacks functionality...\n";
    
    try {
        // Initialize the attack tables
        UltraAttacks::initialize();
        std::cout << "✓ UltraAttacks initialized\n";
        
        // Test simple bishop attack
        uint64_t occupancy = 0ULL;
        uint64_t attacks = UltraAttacks::bishop(28, occupancy); // e4
        std::cout << "✓ Bishop attacks from e4: " << UltraEngine::popcount(attacks) << " squares\n";
        
        // Test simple rook attack
        attacks = UltraAttacks::rook(28, occupancy); // e4
        std::cout << "✓ Rook attacks from e4: " << UltraEngine::popcount(attacks) << " squares\n";
        
        // Test simple queen attack
        attacks = UltraAttacks::queen(28, occupancy); // e4
        std::cout << "✓ Queen attacks from e4: " << UltraEngine::popcount(attacks) << " squares\n";
        
        std::cout << "✅ UltraAttacks test passed!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown exception!" << std::endl;
        return 1;
    }
    
    return 0;
}