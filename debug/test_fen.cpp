#include "../src/position.hpp"
#include "../src/init.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    std::cout << "=== FEN Testing ===" << std::endl;
    
    init();
    Position pos;
    
    // Test the WAC.1 FEN from the image
    std::string wac1_fen = "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - -";
    std::cout << "Testing FEN: " << wac1_fen << std::endl;
    
    if (pos.set_from_fen(wac1_fen)) {
        std::cout << "✓ WAC.1 FEN loaded successfully!" << std::endl;
        std::cout << "Position: " << pos.to_fen() << std::endl;
    } else {
        std::cout << "✗ Failed to load WAC.1 FEN" << std::endl;
        
        // Try with full FEN including move counters
        std::string wac1_full = "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1";
        std::cout << "Trying with full FEN: " << wac1_full << std::endl;
        
        if (pos.set_from_fen(wac1_full)) {
            std::cout << "✓ WAC.1 full FEN loaded successfully!" << std::endl;
            std::cout << "Position: " << pos.to_fen() << std::endl;
        } else {
            std::cout << "✗ Failed to load WAC.1 full FEN" << std::endl;
        }
    }
    
    return 0;
}
