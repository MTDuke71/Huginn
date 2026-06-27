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

/// @brief Maximum PV line length (VICE uses 64).
constexpr int MAX_DEPTH = 64;

/// @brief One PV-table slot: the best move found for a position (VICE PVENTRY).
struct PVEntry {
    uint64_t position_key;  ///< Zobrist key identifying the position.
    S_MOVE move;            ///< Best move found for it.

    PVEntry() : position_key(0), move() {}
};

/**
 * @brief Hash table of best moves keyed by Zobrist position key (VICE PVTABLE).
 *        Used to seed move ordering and to reconstruct the PV for UCI output.
 *        Single-entry-per-index with replace-on-collision.
 */
class PVTable {
private:
    std::vector<PVEntry> entries;  ///< Slot array (power-of-two-ish, size_mb-derived).
    size_t table_size;             ///< Number of slots.

public:
    /// @brief Construct with @p size_mb megabytes of slots (default 2 MB).
    PVTable(size_t size_mb = 2);

    /// @brief Reset every slot to empty (key and move zeroed).
    void clear();

    /// @brief Number of slots in the table.
    size_t size() const { return table_size; }

    /// @brief Store @p move as the best move for @p position_key (replace on collision).
    void store_move(uint64_t position_key, const S_MOVE& move);

    /// @brief Look up the stored move for @p position_key.
    /// @param[out] move Set to the stored move on a key match.
    /// @return true on a hit (key matches), false otherwise.
    bool probe_move(uint64_t position_key, S_MOVE& move) const;

    /// @brief Map a Zobrist key to a slot index (`key % table_size`).
    size_t get_index(uint64_t position_key) const;
};

} // namespace Huginn
