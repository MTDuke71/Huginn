#include "../src/evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>

int main() {
    std::cout << "Testing Specific King Walking Scenarios\n";
    std::cout << "======================================\n\n";
    
    // Test the exact scenario mentioned: f6, Ke7, Ke6, Kf7, Kg6
    
    // 1. After f6 (Black pawn to f6)
    Position pos1;
    pos1.set_from_fen("rnbqkbnr/ppppp1pp/5p2/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 2");
    int eval1 = Engine3::HybridEvaluator::evaluate(pos1);
    std::cout << "After 1...f6: " << eval1 << "cp\n";
    
    // 2. After Ke7 (King to e7)
    Position pos2;
    pos2.set_from_fen("rnbq1bnr/ppppkppp/5p2/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 1 3");
    int eval2 = Engine3::HybridEvaluator::evaluate(pos2);
    std::cout << "After 2...Ke7: " << eval2 << "cp\n";
    
    // 3. After Ke6 (King to e6)
    Position pos3;
    pos3.set_from_fen("rnbq1bnr/pppp1ppp/4kp2/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 2 4");
    int eval3 = Engine3::HybridEvaluator::evaluate(pos3);
    std::cout << "After 3...Ke6: " << eval3 << "cp\n";
    
    // 4. After Kf7 (King to f7)
    Position pos4;
    pos4.set_from_fen("rnbq1bnr/pppp1kpp/5p2/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 3 5");
    int eval4 = Engine3::HybridEvaluator::evaluate(pos4);
    std::cout << "After 4...Kf7: " << eval4 << "cp\n";
    
    // 5. After Kg6 (King to g6)
    Position pos5;
    pos5.set_from_fen("rnbq1bnr/pppp2pp/5pk1/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 4 6");
    int eval5 = Engine3::HybridEvaluator::evaluate(pos5);
    std::cout << "After 5...Kg6: " << eval5 << "cp\n\n";
    
    // Compare to normal development
    Position normal;
    normal.set_from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    int eval_normal = Engine3::HybridEvaluator::evaluate(normal);
    std::cout << "Normal position after 1.e4: " << eval_normal << "cp\n\n";
    
    std::cout << "Evaluation differences from normal:\n";
    std::cout << "f6: " << (eval1 - eval_normal) << "cp penalty\n";
    std::cout << "Ke7: " << (eval2 - eval_normal) << "cp penalty\n";
    std::cout << "Ke6: " << (eval3 - eval_normal) << "cp penalty\n";
    std::cout << "Kf7: " << (eval4 - eval_normal) << "cp penalty\n";
    std::cout << "Kg6: " << (eval5 - eval_normal) << "cp penalty\n";
    
    return 0;
}


