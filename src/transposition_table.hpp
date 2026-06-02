/**
 * @file transposition_table.hpp
 * @brief High-performance hash table for caching search results and alpha-beta bounds
 * 
 * Implements a transposition table using Zobrist hashing to store and retrieve previously
 * computed search results, providing massive search speedups through position caching
 * and alpha-beta bound management. This is one of the most critical optimizations in
 * modern chess engines, often providing 10x-100x search speedup.
 * 
 * ## Architecture Overview
 * 
 * **Hash Table Design:**
 * - Power-of-2 sizing for fast modulo operations via bit masking
 * - Depth-preferred replacement strategy for optimal cache utilization
 * - 16-byte entries optimized for cache line efficiency
 * - Collision handling through key verification (Zobrist hash comparison)
 * 
 * **Entry Structure (16 bytes):**
 * - Zobrist Key (8 bytes): Position identifier for collision detection
 * - Score (2 bytes): Evaluation result adjusted for search depth
 * - Depth (1 byte): Search depth used to compute the score
 * - Node Type (1 byte): EXACT/LOWER_BOUND/UPPER_BOUND for alpha-beta pruning
 * - Best Move (4 bytes): Optimal move found during search (for move ordering)
 * 
 * **Node Types for Alpha-Beta Pruning:**
 * - EXACT: Score is within alpha-beta window (exact evaluation)
 * - LOWER_BOUND: Beta cutoff occurred (score >= beta, actual score may be higher)
 * - UPPER_BOUND: Alpha never improved (score <= alpha, actual score may be lower)
 * 
 * **Performance Characteristics:**
 * - Memory: Default 64MB (4M entries), configurable from 1MB to 2GB
 * - Hit Rate: Typically 95%+ in middle game positions
 * - Search Speedup: 10x-100x depending on search depth and position complexity
 * - Cache Efficiency: Optimized for modern CPU cache hierarchies
 * 
 * **Mate Score Handling:**
 * Mate scores are adjusted by ply distance to ensure accurate mate-in-N calculations
 * when positions are retrieved from different search depths.
 * 
 * @author MTDuke71
 * @version 1.2
 * @see VICE Tutorial Part 84 for algorithmic foundation
 * @see docs/TRANSPOSITION_TABLE_DESIGN.md for detailed implementation notes
 */
#pragma once

#include <cstdint>
#include <vector>
#include <atomic>

// Diagnostic-counter gate, mirrored from search.hpp so transposition_table.hpp
// stays self-contained (no #include of search.hpp would create a cycle since
// search.hpp already depends on this header). When 0, hits/misses/writes are
// neither incremented nor reported via get_hits()/get_writes() — they read as
// the always-zero ATOMIC defaults — saving 3 atomic ops per TT probe/store on
// the hot path. The CLI `-DENABLE_INFO_DIAGNOSTICS=1` flag flips both files
// uniformly because each #ifndef-guards its own redefinition.
#ifndef ENABLE_INFO_DIAGNOSTICS
#define ENABLE_INFO_DIAGNOSTICS 0
#endif

/**
 * @struct TTEntry
 * @brief Transposition table entry storing complete search result information
 * 
 * Each entry stores the complete result of a search at a specific position,
 * including the evaluation score, search depth, node type for alpha-beta
 * pruning, and the best move found. The 16-byte size is optimized for
 * cache efficiency and memory alignment.
 */
struct TTEntry {
    uint64_t zobrist_key;    ///< Zobrist hash for position identification and collision detection
    int16_t score;           ///< Evaluation score (adjusted for mate distance when stored)
    uint8_t depth;           ///< Search depth used to compute this result
    uint8_t node_type;       ///< Node type: EXACT, LOWER_BOUND, or UPPER_BOUND for pruning
    uint32_t best_move;      ///< Best move found during search (encoded S_MOVE format)
    
    /// Node types for alpha-beta bounds management
    static constexpr uint8_t EXACT = 0;        ///< Exact score within alpha-beta window
    static constexpr uint8_t LOWER_BOUND = 1;  ///< Beta cutoff (score >= beta)
    static constexpr uint8_t UPPER_BOUND = 2;  ///< Alpha never improved (score <= alpha)
    
    /// Default constructor initializes empty entry
    TTEntry() : zobrist_key(0), score(0), depth(0), node_type(EXACT), best_move(0) {}
};

/**
 * @class TranspositionTable
 * @brief High-performance hash table for storing and retrieving search results
 * 
 * The transposition table is the primary optimization technique in modern chess engines,
 * providing massive search speedups by caching previously computed position evaluations.
 * Uses Zobrist hashing for position identification and power-of-2 sizing for fast indexing.
 * 
 * ## Key Features:
 * - Fast O(1) average-case lookup and storage
 * - Configurable memory usage (default 64MB)
 * - Statistics tracking for performance analysis
 * - Mate score adjustment for accurate mate-in-N detection
 * - Cache-friendly design with 16-byte aligned entries
 */
class TranspositionTable {
private:
    std::vector<TTEntry> table;    ///< Main hash table storage
    size_t size_mask;              ///< Bit mask for fast modulo (table.size() - 1)
    
    // Statistics tracking for performance analysis
    mutable uint64_t hits = 0;      // Successful probes
    mutable uint64_t misses = 0;    // Failed probes  
    uint64_t writes = 0;            // Store operations
    
public:
    explicit TranspositionTable(size_t size_mb = 64) {
        resize_mb(size_mb);
    }

    // Resize the table to the requested size in MB (UCI "Hash" option).
    // Rounds down to the nearest power of 2 of entries for fast masking, and
    // discards all existing entries. Clamped to at least 1 entry.
    void resize_mb(size_t size_mb) {
        // Calculate number of entries for given size in MB
        size_t num_entries = (size_mb * 1024 * 1024) / sizeof(TTEntry);

        // Round down to nearest power of 2 for fast indexing (minimum 1 entry)
        size_t power_of_2 = 1;
        while (power_of_2 * 2 <= num_entries) {
            power_of_2 *= 2;
        }

        table.assign(power_of_2, TTEntry());  // resize + zero-initialize
        size_mask = power_of_2 - 1;
        hits = misses = writes = 0;
    }
    
    // Store position in transposition table with depth-preferred replacement
    void store(uint64_t zobrist_key, int score, uint8_t depth, uint8_t node_type, uint32_t best_move = 0) {
        size_t index = zobrist_key & size_mask;
        TTEntry& entry = table[index];
        
        // Depth-preferred replacement strategy:
        // Replace if: 1) Empty slot, 2) Same position, 3) New search is deeper, or 4) Significantly older entry
        bool should_replace = false;
        
        if (entry.zobrist_key == 0) {
            // Empty slot - always replace
            should_replace = true;
        } else if (entry.zobrist_key == zobrist_key) {
            // Same position - always update with new information
            should_replace = true;
        } else if (depth >= entry.depth) {
            // New search is deeper or equal - prefer deeper searches
            should_replace = true;
        } else {
            // Keep existing deeper entry, but could add aging logic here in future
            should_replace = false;
        }
        
        if (should_replace) {
            entry.zobrist_key = zobrist_key;
            entry.score = score;
            entry.depth = depth;
            entry.node_type = node_type;
            entry.best_move = best_move;
#if ENABLE_INFO_DIAGNOSTICS
            writes++;  // Track write operations
#endif
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
#if ENABLE_INFO_DIAGNOSTICS
            hits++;  // Track successful probes
#endif
            return true;
        }
#if ENABLE_INFO_DIAGNOSTICS
        misses++;  // Track failed probes
#endif
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
    
    size_t get_size() const { return table.size(); }

    // UCI standard `hashfull` — fraction of TT in use expressed in permill
    // (0..1000). Loop is O(N) but only called once per iterative-deepening
    // depth (one info line per depth), not per node. Always available
    // regardless of the diagnostics gate; UCI GUIs use this to colour
    // time-pressure indicators and decide adjudication.
    int permill_full() const {
        const size_t n = table.size();
        if (n == 0) return 0;
        // Sample-based estimate to keep cost negligible on huge TTs (e.g. 256MB).
        // Walk the first 1000 entries; spec only requires accuracy to ~1‰.
        const size_t sample = n < 1000 ? n : 1000;
        size_t filled = 0;
        for (size_t i = 0; i < sample; ++i) {
            if (table[i].zobrist_key != 0) ++filled;
        }
        return static_cast<int>((filled * 1000ULL) / sample);
    }

    // Get statistics
    uint64_t get_hits() const { return hits; }
    uint64_t get_writes() const { return writes; }
};
