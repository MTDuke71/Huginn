#include "../Engine3_src/hybrid_evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>

int main() {
    std::cout << "Testing King Safety Evaluation\n";
    std::cout << "==============================\n\n";
    
    // Test position with king on g6 (from the game)
    Position pos1;
    pos1.set_from_fen("r1bqkbnr/ppp3pp/2P2pk1/8/2BP4/5N2/PPP2PPP/R1BQK2R b KQkq - 0 7");
    
    int eval1 = Engine3::HybridEvaluator::evaluate(pos1);
    std::cout << "Position with Black king on g6: " << eval1 << "cp\n";
    
    // Test similar position but with king safely on e8  
    Position pos2;
    pos2.set_from_fen("r1bqkbnr/ppp3pp/2P5/8/2BP4/5N2/PPP2PPP/R1BQK2R b KQkq - 0 7");
    
    int eval2 = Engine3::HybridEvaluator::evaluate(pos2);
    std::cout << "Position with Black king on e8: " << eval2 << "cp\n";
    
    std::cout << "\nDifference: " << (eval1 - eval2) << "cp\n";
    std::cout << "King safety penalty: " << (eval2 - eval1) << "cp\n\n";
    
    if (abs(eval2 - eval1) < 200) {
        std::cout << "WARNING: King safety penalty is too small!\n";
        std::cout << "The engine doesn't understand the king is in mortal danger!\n";
    } else {
        std::cout << "King safety evaluation seems adequate.\n";
    }
    
    return 0;
}


