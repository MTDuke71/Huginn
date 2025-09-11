/**
 * @file polyglot_book.hpp
 * @brief Opening book support using the standard Polyglot book format
 * 
 * Implements loading and querying of chess opening books in the widely-used Polyglot
 * binary format (.bin files). Polyglot books contain collections of opening moves
 * with statistical weights based on master game databases, allowing the engine to
 * play strong, varied openings without extensive opening theory programming.
 * 
 * ## Polyglot Format Overview
 * 
 * **File Structure:**
 * - Binary file with fixed 16-byte entries, sorted by position key
 * - Each entry contains: position hash (8 bytes) + move (2 bytes) + weight (2 bytes) + learn (4 bytes)
 * - Positions can have multiple move entries with different weights
 * - Binary search enables fast O(log n) position lookup
 * 
 * **Position Hashing:**
 * - Uses Polyglot-specific Zobrist hashing (different from engine's internal hashing)
 * - Includes piece placement, side to move, castling rights, en passant
 * - Provides unique 64-bit identifier for each chess position
 * 
 * **Move Format:**
 * - Polyglot uses 16-bit move encoding: from(6) + to(6) + promotion(4)
 * - Requires conversion to/from engine's internal S_MOVE format
 * - Handles all move types including castling, en passant, and promotions
 * 
 * **Weight System:**
 * - Higher weights indicate more frequently played moves in master games
 * - Engine can select moves probabilistically based on weights
 * - Supports deterministic (best move) or varied (weighted random) play styles
 * 
 * ## Integration with Engine
 * 
 * The book is consulted before starting the main search algorithm:
 * 1. Check if current position exists in opening book
 * 2. If found, select move based on weights (best or random)
 * 3. If not found or book disabled, proceed with normal search
 * 
 * **Performance:**
 * - Fast binary search: O(log n) lookup time
 * - Memory efficient: entries loaded on demand or cached
 * - Typical book sizes: 1MB-100MB (10K-1M positions)
 * 
 * @author MTDuke71
 * @version 1.1
 * @see https://www.chessprogramming.org/Polyglot for format specification
 * @see docs/OPENING_BOOK_INTEGRATION.md for usage examples
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include "move.hpp"
#include "position.hpp"

namespace Huginn {

/**
 * @struct PolyglotEntry
 * @brief Single entry in a Polyglot opening book (exactly 16 bytes)
 * 
 * Represents one move option for a specific chess position, including the move
 * itself and its frequency/strength based on master game statistics. The fixed
 * 16-byte size matches the binary format used by Polyglot book files.
 */
struct PolyglotEntry {
    uint64_t key;      ///< Polyglot position hash key (8 bytes) - unique position identifier
    uint16_t move;     ///< Move in Polyglot 16-bit format (2 bytes): from(6)+to(6)+promotion(4)
    uint16_t weight;   ///< Move weight/frequency from master games (2 bytes) - higher = more popular
    uint32_t learn;    ///< Learning value (4 bytes) - typically unused in modern implementations
    
    /// Default constructor creates empty entry
    PolyglotEntry() : key(0), move(0), weight(0), learn(0) {}
    
    /// Constructor for creating book entries with position, move, and weight
    PolyglotEntry(uint64_t k, uint16_t m, uint16_t w) : key(k), move(m), weight(w), learn(0) {}
};

/**
 * @class PolyglotBook
 * @brief Manager for Polyglot format opening books
 * 
 * Handles loading, parsing, and querying of Polyglot opening book files (.bin format).
 * Provides both deterministic (strongest move) and probabilistic (weighted random)
 * move selection from opening theory. Books are loaded once and kept in memory for
 * fast position lookup during gameplay.
 * 
 * ## Usage Pattern:
 * ```cpp
 * PolyglotBook book;
 * if (book.load("book.bin")) {
 *     if (S_MOVE move = book.get_book_move(position); move.move != 0) {
 *         // Use book move instead of searching
 *     }
 * }
 * ```
 */
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
