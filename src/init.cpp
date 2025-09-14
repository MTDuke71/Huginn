/**
 * @file init.cpp
 * @brief Engine initialization implementation
 * 
 * Implements the initialization functions for the Huginn chess engine.
 * Handles one-time setup of critical engine subsystems including hash tables,
 * evaluation masks, and other global state that must be initialized before
 * the engine can operate correctly.
 * 
 * ## Initialization Order
 * 1. **Zobrist Tables**: Position hashing for transposition table
 * 2. **Evaluation Masks**: Precomputed patterns for position evaluation
 * 3. **Global State**: Engine-wide flags and configuration
 * 
 * ## Thread Safety
 * The initialization functions are not thread-safe and should only be called
 * once during program startup before any multithreaded operations begin.
 * 
 * @author MTDuke71
 * @version 1.2
 * @see init.hpp for function declarations
 */
#include "init.hpp"
#include "zobrist.hpp"
#include "evaluation.hpp"
#include "knight_lookup_tables.hpp"
#include "king_lookup_tables.hpp"

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
        
        // Initialize knight lookup tables for optimized move generation
        KnightLookupTables::initialize_knight_tables();
        
        // Initialize king lookup tables for optimized move generation
        #ifdef USE_KING_LOOKUP_TABLES
        KingLookupTables::initialize_king_tables();
        #endif
        
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
