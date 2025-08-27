#include "../src/hybrid_evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>

int main() {
    std::cout << "Huginn Opening Move Analyzer\n";
    std::cout << "============================\n\n";
    
    try {
        // Analyze all opening moves using static evaluation
        Evaluation::analyze_opening_moves(8);
    } catch (const std::exception& e) {
        std::cerr << "Error during analysis: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}


