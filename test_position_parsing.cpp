#include "src/position.hpp"
#include "src/init.hpp"
#include <iostream>

int main() {
    Huginn::init();
    
    Position pos;
    
    std::cout << "Testing position parsing...\n";
    
    // Test parsing starting position
    bool success = pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    if (success) {
        std::cout << "Successfully parsed starting position\n";
    } else {
        std::cout << "Failed to parse starting position\n";
        return 1;
    }
    
    return 0;
}