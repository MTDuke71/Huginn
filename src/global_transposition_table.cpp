#include "global_transposition_table.hpp"
#include <stdexcept>

namespace Huginn {
    // Global transposition table instance
    std::unique_ptr<TranspositionTable> g_transposition_table;
    
    void init_global_transposition_table(size_t size_mb) {
        // Clean up any existing table first
        cleanup_global_transposition_table();
        
        // Create new global table
        g_transposition_table = std::make_unique<TranspositionTable>(size_mb);
    }
    
    TranspositionTable& get_transposition_table() {
        if (!g_transposition_table) {
            throw std::runtime_error("Global transposition table not initialized. Call init_global_transposition_table() first.");
        }
        return *g_transposition_table;
    }
    
    void cleanup_global_transposition_table() {
        g_transposition_table.reset();
    }
    
    void increment_tt_age() {
        if (g_transposition_table) {
            g_transposition_table->increment_age();
        }
    }
    
    void reset_tt_age() {
        if (g_transposition_table) {
            g_transposition_table->reset_age();
        }
    }
}