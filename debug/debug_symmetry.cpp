#include "../src/hybrid_evaluation.hpp"
#include "../src/simple_search.hpp"
#include "position.hpp"
#include "init.hpp"
#include <iostream>

int main() {
    Huginn::init();
    
    std::cout << "Analyzing Endgame Position Symmetry Issue\n";
    std::cout << "=========================================\n\n";
    
    // Original position
    Position pos1;
    pos1.set_from_fen("8/2k1p3/3p4/3P4/3K4/8/8/8 w - - 0 1");
    
    // Mirrored position  
    Position pos2;
    pos2.set_from_fen("8/8/8/3k4/3p4/3P4/2K1P3/8 b - - 0 1");
    
    std::cout << "ORIGINAL POSITION (White to move):\n";
    std::cout << "FEN: 8/2k1p3/3p4/3P4/3K4/8/8/8 w - - 0 1\n";
    std::cout << "Total: " << Engine3::HybridEvaluator::evaluate(pos1) << "cp\n";
    std::cout << "(Engine3 uses unified evaluation - individual components not exposed)\n\n";
    
    std::cout << "MIRRORED POSITION (Black to move):\n";
    std::cout << "FEN: 8/8/8/3k4/3p4/3P4/2K1P3/8 b - - 0 1\n";
    std::cout << "Total: " << Engine3::HybridEvaluator::evaluate(pos2) << "cp\n";
    std::cout << "(Engine3 uses unified evaluation - individual components not exposed)\n\n";
    
    std::cout << "DIFFERENCES:\n";
    std::cout << "Total Difference: " << (Engine3::HybridEvaluator::evaluate(pos1) - Engine3::HybridEvaluator::evaluate(pos2)) << "cp\n";
    
    return 0;
}

