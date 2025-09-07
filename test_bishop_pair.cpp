#include <iostream>
#include "src/position.hpp"
#include "src/minimal_search.hpp"

using namespace Huginn;

int main() {
    MinimalEngine engine;
    
    // Position with bishop pair for white
    std::string fen_with_pair = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    // Position without bishop pair (one bishop traded)
    std::string fen_without_pair = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RN1QKBNR w KQkq - 0 1";
    
    Position pos_with, pos_without;
    pos_with.set_from_fen(fen_with_pair);
    pos_without.set_from_fen(fen_without_pair);
    
    int eval_with = engine.evaluate(pos_with);
    int eval_without = engine.evaluate(pos_without);
    
    std::cout << "=== Bishop Pair Bonus Test ===" << std::endl;
    std::cout << "Position with bishop pair: " << eval_with << " cp" << std::endl;
    std::cout << "Position without bishop pair: " << eval_without << " cp" << std::endl;
    std::cout << "Difference: " << (eval_with - eval_without) << " cp" << std::endl;
    std::cout << "Expected difference: 50 cp (bishop pair bonus)" << std::endl;
    
    if (abs((eval_with - eval_without) - 50) <= 5) {
        std::cout << "✅ Bishop pair bonus working correctly!" << std::endl;
    } else {
        std::cout << "❌ Bishop pair bonus not working as expected" << std::endl;
    }
    
    return 0;
}
