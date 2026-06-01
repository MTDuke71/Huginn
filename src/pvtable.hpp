/**
 * @file pvtable.hpp
 * @brief Principal Variation Table implementation
 * 
 * Implements a hash table for storing and retrieving principal variation (PV) moves
 * during search. The PV table maintains the best line of play found during search,
 * enabling the engine to reconstruct and display the principal variation for analysis.
 * The implementation follows the VICE tutorial approach adapted for modern C++.
 * 
 * ## PV Table Architecture
 * - **Hash-based Storage**: Zobrist position keys for fast lookup
 * - **Move Storage**: Best move for each position in the principal variation
 * - **Dynamic Sizing**: Configurable table size for memory management
 * - **Collision Handling**: Simple replacement scheme for hash collisions
 * 
 * ## Search Integration
 * - **Move Ordering**: PV moves get highest priority in move ordering
 * - **Search Guidance**: Previous PV guides iterative deepening search
 * - **Analysis Output**: PV reconstruction for UCI output
 * - **Time Management**: PV stability influences time allocation
 * 
 * ## Performance Features
 * - Fast hash-based position lookup
 * - Minimal memory overhead per entry
 * - Cache-friendly data layout
 * - Integration with transposition table system
 * 
 * @author MTDuke71
 * @version 1.2
 * @see search.hpp for search engine integration
 * @see move.hpp for move representation
 */

#pragma once
#include "move.hpp"
#include <vector>
#include <cstdint>

// Forward declaration
class Position;

namespace Huginn {

// MAX_DEPTH for PV line (tutorial uses 64)
constexpr int MAX_DEPTH = 64;

// Equivalent to PVENTRY from VICE tutorial (1:30)
struct PVEntry {
    uint64_t position_key;  // Zobrist key for this position
    S_MOVE move;           // Best move for this position
    
    PVEntry() : position_key(0), move() {}
};

// Equivalent to PVTABLE from VICE tutorial (2:05) 
class PVTable {
private:
    std::vector<PVEntry> entries;  // Dynamic array of PV entries
    size_t table_size;             // Current size of table
    
public:
    // Constructor - initialize with default 2MB size (3:35 in tutorial)
    PVTable(size_t size_mb = 2);
    
    // Clear the table - set all keys and moves to zero (6:04 in tutorial)
    void clear();
    
    // Get table size information
    size_t size() const { return table_size; }
    size_t memory_usage_mb() const { return (table_size * sizeof(PVEntry)) / (1024 * 1024); }
    
    // Store a move in the PV table
    void store_move(uint64_t position_key, const S_MOVE& move);
    
    // Probe the PV table for a move
    bool probe_move(uint64_t position_key, S_MOVE& move) const;
    
    // Get index from position key (hash function)
    size_t get_index(uint64_t position_key) const;
};

} // namespace Huginn
