// init.cpp
#include "init.hpp"
#include "zobrist.hpp"
#include "attacks.hpp"
// Add other includes here as more subsystems are added
// #include "bitboard.hpp"  // if bitboard needs initialization
// #include "movegen.hpp"   // if move generation tables need initialization

namespace Huginn {
    
    static bool initialized = false;
    
    void init() {
        if (initialized) {
            return;  // Already initialized, avoid duplicate work
        }
        
        // Initialize Zobrist hashing tables
        Zobrist::init_zobrist();
        Attacks::init();
        
        // TODO: Add other subsystem initializations here as needed:
        // - Evaluation tables  
        // - Opening book
        // - Endgame tablebase paths
        // - Search parameters
        
        initialized = true;
    }
    
    bool is_initialized() {
        return initialized;
    }
    
} // namespace Huginn