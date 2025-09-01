#include "pvtable.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
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

// MoveExists function - check if move is legal (Part 53 - 1:04-2:28)
bool PVTable::move_exists(Position& pos, const S_MOVE& move) const {
    if (move.move == 0) return false;  // Invalid move
    
    // Generate all legal moves
    S_MOVELIST move_list;
    generate_legal_moves_enhanced(pos, move_list);
    
    // Check if the move exists in the legal move list
    for (int i = 0; i < move_list.count; ++i) {
        if (move_list.moves[i].move == move.move) {
            return true;  // Move is legal
        }
    }
    
    return false;  // Move not found - illegal
}

// GetPVLine function - retrieve PV line from table (Part 53 - 2:29-6:40)
int PVTable::get_pv_line(Position& pos, int depth, S_MOVE pv_array[MAX_DEPTH]) {
    int count = 0;
    
    // Probe PV table for moves up to desired depth
    while (count < depth && count < MAX_DEPTH) {
        S_MOVE move;
        
        // Probe table for best move at current position
        if (!probe_move(pos.zobrist_key, move)) {
            break;  // No move found in table
        }
        
        // Check if move is legal (crucial due to hash collisions)
        if (!move_exists(pos, move)) {
            break;  // Illegal move - probably hash collision
        }
        
        // Store move in PV array
        pv_array[count] = move;
        count++;
        
        // Make the move to get to next position
        if (pos.MakeMove(move) != 1) {
            break;  // Move failed to make
        }
    }
    
    // Take back all moves to restore original position (Part 53 - 6:40)
    for (int i = 0; i < count; ++i) {
        pos.TakeMove();
    }
    
    return count;  // Number of moves in PV line
}

} // namespace Huginn
