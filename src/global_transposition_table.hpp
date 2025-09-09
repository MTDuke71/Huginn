#pragma once

#include "transposition_table.hpp"
#include <memory>

namespace Huginn {
    // Global transposition table instance for lazy SMP
    extern std::unique_ptr<TranspositionTable> g_transposition_table;
    
    /**
     * @brief Initialize the global transposition table
     * @param size_mb Size of the table in megabytes (default: 64MB)
     * 
     * This creates a single shared transposition table that all search threads
     * can access. This is essential for lazy SMP where multiple threads share
     * search results through a common hash table.
     */
    void init_global_transposition_table(size_t size_mb = 64);
    
    /**
     * @brief Get reference to the global transposition table
     * @return Reference to the global transposition table
     * @throws std::runtime_error if table hasn't been initialized
     * 
     * This function provides thread-safe access to the global table.
     * All MinimalEngine instances should use this instead of their own table.
     */
    TranspositionTable& get_transposition_table();
    
    /**
     * @brief Clean up the global transposition table
     * 
     * Safely destroys the global table and releases memory.
     * Should be called during engine shutdown.
     */
    void cleanup_global_transposition_table();
    
    /**
     * @brief Increment age for new search (VICE Part 85)
     * 
     * Should be called at the start of each new search to improve
     * replacement strategy and prevent truncated principal variations.
     */
    void increment_tt_age();
    
    /**
     * @brief Reset age for new game (VICE Part 85)
     * 
     * Should be called when starting a new game.
     */
    void reset_tt_age();
    
    /**
     * @brief Clear transposition table statistics for new game
     * 
     * Clears hit/miss/write counters while preserving hash entries.
     * Should be called when starting a new game for accurate per-game statistics.
     */
    void clear_tt_stats();
}
