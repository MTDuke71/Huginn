#include "pvtable.hpp"
#include <iostream>

namespace Huginn {

// Constructor - allocate memory for PV table (2:40 in tutorial)
PVTable::PVTable(size_t size_mb) {
    // Calculate number of entries for given memory size
    size_t bytes = size_mb * 1024 * 1024;
    table_size = bytes / sizeof(PVEntry);
    
    // Dynamic allocation (2:40) - using vector for RAII safety
    entries.resize(table_size);
    
    // Initialize by clearing (tutorial shows this in init)
    clear();
    
    std::cout << "PV Table initialized: " << table_size << " entries, " 
              << memory_usage_mb() << " MB" << std::endl;
}

// Clear the table - set all keys and moves to zero (6:04 in tutorial)
void PVTable::clear() {
    for (auto& entry : entries) {
        entry.position_key = 0;
        entry.move = S_MOVE();  // Clear move
    }
}

// Simple hash function to get index from position key
size_t PVTable::get_index(uint64_t position_key) const {
    // Simple modulo hash - tutorial would use similar approach
    return position_key % table_size;
}

// Store a move in the PV table
void PVTable::store_move(uint64_t position_key, const S_MOVE& move) {
    size_t index = get_index(position_key);
    
    entries[index].position_key = position_key;
    entries[index].move = move;
}

// Probe the PV table for a move
bool PVTable::probe_move(uint64_t position_key, S_MOVE& move) const {
    size_t index = get_index(position_key);
    
    if (entries[index].position_key == position_key) {
        move = entries[index].move;
        return true;  // Hit
    }
    
    return false;  // Miss
}

} // namespace Huginn
