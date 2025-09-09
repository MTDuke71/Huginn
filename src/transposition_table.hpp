#pragma once

#include <cstdint>
#include <vector>
#include <atomic>

// Transposition Table entry for storing search results
// Thread-Safety: Safe for concurrent access in lazy SMP
// - Multiple threads can safely read different entries
// - Multiple threads can write to same entry (any valid entry is beneficial)
// - No data corruption occurs from race conditions
struct TTEntry {
    uint64_t zobrist_key;    // Position identifier
    int16_t score;           // Evaluation score
    uint8_t depth;           // Search depth 
    uint8_t node_type;       // EXACT, LOWER_BOUND, UPPER_BOUND
    uint8_t age;             // Age for replacement strategy (VICE Part 85)
    uint32_t best_move;      // Best move found (encoded)
    
    // Node types for alpha-beta bounds
    static constexpr uint8_t EXACT = 0;
    static constexpr uint8_t LOWER_BOUND = 1; // Alpha cutoff
    static constexpr uint8_t UPPER_BOUND = 2; // Beta cutoff
    
    TTEntry() : zobrist_key(0), score(0), depth(0), node_type(EXACT), age(0), best_move(0) {}
};

/**
 * @brief Transposition table for caching search results
 * 
 * Thread-Safety for Lazy SMP:
 * - Designed for concurrent access by multiple search threads
 * - Store operations are safe even with write collisions
 * - Probe operations are always safe for reading
 * - Statistics may be approximate in multi-threaded environment
 */
class TranspositionTable {
private:
    std::vector<TTEntry> table;
    size_t size_mask;  // For fast modulo (size must be power of 2)
    uint8_t current_age = 0;  // Current age for replacement strategy (VICE Part 85)
    
    // Statistics tracking
    mutable uint64_t hits = 0;      // Successful probes
    mutable uint64_t misses = 0;    // Failed probes  
    uint64_t writes = 0;            // Store operations
    
public:
    explicit TranspositionTable(size_t size_mb = 64) {
        // Calculate number of entries for given size in MB
        size_t num_entries = (size_mb * 1024 * 1024) / sizeof(TTEntry);
        
        // Round down to nearest power of 2 for fast indexing
        size_t power_of_2 = 1;
        while (power_of_2 * 2 <= num_entries) {
            power_of_2 *= 2;
        }
        
        table.resize(power_of_2);
        size_mask = power_of_2 - 1;
    }
    
    // Store position in transposition table with age-based replacement (VICE Part 85)
    void store(uint64_t zobrist_key, int score, uint8_t depth, uint8_t node_type, uint32_t best_move = 0) {
        size_t index = zobrist_key & size_mask;
        TTEntry& entry = table[index];
        
        // Age-based replacement strategy:
        // Replace if:
        // 1. Entry is empty (zobrist_key == 0)
        // 2. Same position (zobrist_key matches)
        // 3. Entry is from an older age (less recent search)
        // 4. Entry is from same age but new depth is deeper
        bool should_replace = (entry.zobrist_key == 0) ||                    // Empty slot
                             (entry.zobrist_key == zobrist_key) ||            // Same position
                             (entry.age < current_age) ||                     // Older entry
                             (entry.age == current_age && depth >= entry.depth); // Same age, deeper or equal depth
        
        if (should_replace) {
            entry.zobrist_key = zobrist_key;
            entry.score = score;
            entry.depth = depth;
            entry.node_type = node_type;
            entry.age = current_age;
            entry.best_move = best_move;
            
            writes++;  // Track write operations
        }
    }
    
    // Probe transposition table for position
    bool probe(uint64_t zobrist_key, int& score, uint8_t& depth, uint8_t& node_type, uint32_t& best_move) const {
        size_t index = zobrist_key & size_mask;
        const TTEntry& entry = table[index];
        
        if (entry.zobrist_key == zobrist_key) {
            score = entry.score;
            depth = entry.depth;
            node_type = entry.node_type;
            best_move = entry.best_move;
            hits++;  // Track successful probes
            return true;
        }
        misses++;  // Track failed probes
        return false;
    }
    
    // Clear all entries
    void clear() {
        for (auto& entry : table) {
            entry = TTEntry();
        }
        // Reset statistics and age
        hits = misses = writes = 0;
        current_age = 0;
    }
    
    // Get table utilization statistics
    double get_utilization() const {
        size_t filled = 0;
        for (const auto& entry : table) {
            if (entry.zobrist_key != 0) filled++;
        }
        return double(filled) / table.size();
    }
    
    size_t get_size() const { return table.size(); }
    
    // Get statistics
    uint64_t get_hits() const { return hits; }
    uint64_t get_misses() const { return misses; }
    uint64_t get_writes() const { return writes; }
    uint64_t get_total_probes() const { return hits + misses; }
    double get_hit_rate() const { 
        uint64_t total = get_total_probes();
        return total > 0 ? double(hits) / total : 0.0; 
    }
    
    // Clear statistics only
    void clear_stats() {
        hits = misses = writes = 0;
    }
    
    // VICE Part 85: Age management for better replacement strategy
    
    /**
     * @brief Increment age for new search
     * 
     * Should be called at the start of each new search to mark entries
     * from this search as more recent than previous searches.
     */
    void increment_age() {
        current_age++;
        // Prevent overflow (though 255 searches is quite a lot)
        if (current_age == 0) current_age = 1;
    }
    
    /**
     * @brief Reset age for new game
     * 
     * Should be called when starting a new game to reset the age system.
     */
    void reset_age() {
        current_age = 0;
    }
    
    /**
     * @brief Get current age
     * 
     * @return Current age value
     */
    uint8_t get_age() const {
        return current_age;
    }
};
