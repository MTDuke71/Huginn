// init.cpp
#include "init.hpp"
#include "zobrist.hpp"
#include "evaluation.hpp"  // For VICE Part 78 mask initialization
// Add other includes here as more subsystems are added
// #include "bitboard.hpp"  // if bitboard needs initialization

namespace Huginn {
    
    static bool initialized = false;
    
    void init() {
        if (initialized) {
            return;  // Already initialized, avoid duplicate work
        }
        
        // Initialize Zobrist hashing tables
        Zobrist::init_zobrist();
        
        // Initialize evaluation masks (VICE Part 78)
        EvalParams::init_evaluation_masks();
        
        initialized = true;
    }
    
    bool is_initialized() {
        return initialized;
    }
    
} // namespace Huginn
