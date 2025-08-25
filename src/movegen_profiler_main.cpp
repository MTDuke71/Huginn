#include "movegen_profiler.hpp"
#include "init.hpp"
#include <iostream>

int main() {
    std::cout << "Huginn Chess Engine - Move Generation Profiler" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "Detailed analysis of move generation function performance" << std::endl;
    std::cout << std::endl;
    
    try {
        // Initialize the chess engine
        Huginn::init();
        if (!Huginn::is_initialized()) {
            std::cerr << "Failed to initialize chess engine" << std::endl;
            return 1;
        }
        
        // Run comprehensive profiling
        MoveGenProfiler::profile_comprehensive_movegen();
        
        std::cout << std::endl << "=== PROFILING COMPLETE ===" << std::endl;
        std::cout << "Analysis complete. Review the recommendations above to identify" << std::endl;
        std::cout << "the most effective optimization targets for move generation." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Profiling failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}