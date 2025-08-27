#include "../src/evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>

int main() {
    std::cout << "Huginn Opening Move Analyzer\n";
    std::cout << "============================\n\n";
    
    try {
        // Opening analysis not yet implemented in current architecture
        std::cout << "Opening analysis not yet implemented" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error during analysis: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}


