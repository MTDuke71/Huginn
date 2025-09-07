#pragma once

#include <cstdint>
#include <vector>
#include <atomic>

// Transposition Table entry for storing search results
struct TTEntry {
    uint64_t zobrist_key;    // Position identifier
    int16_t score;           // Evaluation score
    uint8_t depth;           // Search depth 
    uint8_t node_type;       // EXACT, LOWER_BOUND, UPPER_BOUND
    uint32_t best_move;      // Best move found (encoded)
    
    // Node types for alpha-beta bounds
    static constexpr uint8_t EXACT = 0;
    static constexpr uint8_t LOWER_BOUND = 1; // Alpha cutoff
    static constexpr uint8_t UPPER_BOUND = 2; // Beta cutoff
    
    TTEntry() : zobrist_key(0), score(0), depth(0), node_type(EXACT), best_move(0) {}
};

class TranspositionTable {
private:
    std::vector<TTEntry> table;
    size_t size_mask;  // For fast modulo (size must be power of 2)
    
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
    
    // Store position in transposition table
    void store(uint64_t zobrist_key, int score, uint8_t depth, uint8_t node_type, uint32_t best_move = 0) {
        size_t index = zobrist_key & size_mask;
        TTEntry& entry = table[index];
        
        // Always replace strategy (could implement depth-preferred later)
        entry.zobrist_key = zobrist_key;
        entry.score = score;
        entry.depth = depth;
        entry.node_type = node_type;
        entry.best_move = best_move;
        
        writes++;  // Track write operations
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
        // Reset statistics
        hits = misses = writes = 0;
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
};
