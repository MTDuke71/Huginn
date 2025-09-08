#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include "move.hpp"
#include "position.hpp"

namespace Huginn {

// Polyglot book entry structure (exactly 16 bytes)
// This matches the binary format used by Polyglot opening books
struct PolyglotEntry {
    uint64_t key;      // Position hash key (8 bytes)
    uint16_t move;     // Move in Polyglot format (2 bytes) 
    uint16_t weight;   // Move weight/frequency (2 bytes)
    uint32_t learn;    // Learning value (4 bytes) - usually unused
    
    PolyglotEntry() : key(0), move(0), weight(0), learn(0) {}
    PolyglotEntry(uint64_t k, uint16_t m, uint16_t w) : key(k), move(m), weight(w), learn(0) {}
};

class PolyglotBook {
private:
    std::vector<PolyglotEntry> entries;
    bool is_loaded = false;
    std::string book_path;
    
    // Convert internal move to Polyglot format
    uint16_t move_to_polyglot(const S_MOVE& move) const;
    
    // Convert Polyglot move to internal format
    S_MOVE polyglot_to_move(uint16_t poly_move, const Position& pos) const;
    
    // Binary search for entries with matching key
    std::vector<const PolyglotEntry*> find_entries(uint64_t key) const;

public:
    PolyglotBook() = default;
    explicit PolyglotBook(const std::string& path) : book_path(path) {}
    
    // Generate Polyglot hash key for position (made public for debugging)
    uint64_t get_polyglot_key(const Position& pos) const;
    
    // Load opening book from file
    bool load_book(const std::string& path);
    
    // Get book move for position (returns best weighted move)
    S_MOVE get_book_move(const Position& pos) const;
    
    // Get all book moves for position with weights
    std::vector<std::pair<S_MOVE, uint16_t>> get_all_book_moves(const Position& pos) const;
    
    // Check if position is in opening book
    bool has_book_moves(const Position& pos) const;
    
    // Get book statistics
    size_t get_entry_count() const { return entries.size(); }
    bool is_book_loaded() const { return is_loaded; }
    const std::string& get_book_path() const { return book_path; }
    
    // Clear loaded book
    void clear();
};

} // namespace Huginn
