// Principal Variation Table - VICE tutorial style adapted to C++
// This will store the best line of moves found during search

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
    
    // Check if a move exists (is legal) in current position
    bool move_exists(Position& pos, const S_MOVE& move) const;
    
    // Get PV line from table (Part 53 - GetPVLine function)
    int get_pv_line(Position& pos, int depth, S_MOVE pv_array[MAX_DEPTH]);
};

} // namespace Huginn
