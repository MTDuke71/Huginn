// Test if our king table logic creates asymmetries
#include <iostream>
#include "src/position.hpp"
#include "src/minimal_search.hpp"

using namespace Huginn;

void test_king_table_symmetry() {
    std::cout << "=== Testing King Table Symmetry ===" << std::endl;
    
    MinimalEngine engine;
    
    // Test a simple position with just kings and one piece to see if king tables are symmetric
    std::cout << "\nTesting simple king + rook positions:" << std::endl;
    
    Position pos1;
    pos1.set_from_fen("4k3/8/8/8/8/8/8/R3K3 w - - 0 1");  // White king e1, black king e8
    int eval1 = engine.evaluate(pos1);
    std::cout << "White king e1, black king e8: " << eval1 << " cp" << std::endl;
    
    Position pos2;
    pos2.set_from_fen("r3k3/8/8/8/8/8/8/4K3 b - - 0 1");  // Mirrored: black king e1, white king e8
    int eval2 = engine.evaluate(pos2);
    std::cout << "Black king e1, white king e8: " << eval2 << " cp" << std::endl;
    
    std::cout << "Difference: " << (eval1 - eval2) << " cp (should be 0 for symmetry)" << std::endl;
    
    // Test with different king positions to see if table indexing is correct
    std::cout << "\nTesting corner vs center king positions:" << std::endl;
    
    Position corner_white;
    corner_white.set_from_fen("7k/8/8/8/8/8/8/K6R w - - 0 1");  // White king a1
    int corner_eval = engine.evaluate(corner_white);
    std::cout << "White king a1: " << corner_eval << " cp" << std::endl;
    
    Position corner_black;
    corner_black.set_from_fen("k6r/8/8/8/8/8/8/7K b - - 0 1");  // Black king a8 (mirrored)
    int corner_black_eval = engine.evaluate(corner_black);
    std::cout << "Black king a8 (mirrored): " << corner_black_eval << " cp" << std::endl;
    
    std::cout << "Corner difference: " << (corner_eval - corner_black_eval) << " cp" << std::endl;
}

int main() {
    test_king_table_symmetry();
    return 0;
}
