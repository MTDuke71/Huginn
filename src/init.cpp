// init.cpp
#include "init.hpp"
#include "zobrist.hpp"
#include "evaluation.hpp"

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
        
        initialized = true;
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
