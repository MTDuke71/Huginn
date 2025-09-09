// init.cpp
#include "init.hpp"
#include "zobrist.hpp"
#include "evaluation.hpp"
#include "global_transposition_table.hpp"

namespace Huginn {
    
    static bool initialized = false;

    /**
     * @brief Initializes all engine subsystems.
     * 
     * This function must be called once at program startup before using any engine functionality.
     * It sets up all necessary subsystems required for the engine to operate correctly.
     */
    void init() {
        if (initialized) {
            return;  // Already initialized, avoid duplicate work
        }
        
        // Initialize Zobrist hashing tables
        Zobrist::init_zobrist();
        
        // Initialize evaluation masks
        EvalParams::init_evaluation_masks();
        
        // Initialize global transposition table (64MB default)
        // Note: Moved to global for lazy SMP multi-threading support
        init_global_transposition_table(64);
        
        initialized = true;
    }

    /**
     * @brief Cleans up all engine subsystems.
     * 
     * This function should be called at program shutdown to properly clean up resources.
     * It's safe to call this multiple times.
     */
    void cleanup() {
        if (!initialized) {
            return;  // Not initialized, nothing to clean up
        }
        
        // Clean up global transposition table
        cleanup_global_transposition_table();
        
        initialized = false;
    }

    /**
     * @brief Checks if the engine has been properly initialized.
     * 
     * @return true if the engine has been initialized, false otherwise.
     */
    bool is_initialized() {
        return initialized;
    }
    
} // namespace Huginn
