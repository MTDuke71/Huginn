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
 * - Power-of-2 sizing so the table index is `zobrist_key & (size-1)` — a single
 *   AND replaces a modulo on the hot path.
 * - **One entry per index** (no clusters): a store collides directly with
 *   whatever shares its index. Verification stores the *full* 64-bit Zobrist key
 *   and compares it on probe, so a wrong-position hit is effectively impossible
 *   (no separate index/lock split).
 * - 16-byte entries (8 key + 2 score + 1 depth + 1 type + 4 move) for cache-line
 *   friendliness.
 *
 * @par Replacement (and the aging gap)
 * Slots are scarce, so a store must sometimes evict. The base policy is
 * **depth-preferred**: overwrite on an empty slot, the same position, or a new
 * search at `depth >= stored depth`; otherwise keep the existing (deeper) entry.
 * The table is **not cleared between searches** within a game, so without aging
 * a deep entry stored early can squat in its slot for the rest of the game,
 * blocking fresher shallow results. ENABLE_TT_AGING (BACKLOG #42 idea 1) adds
 * date-based aging on top: entries carry a 6-bit search date in the node_type
 * byte, any stale-dated entry is evictable regardless of depth, and probe hits
 * re-date the entry. N-way clusters (#42 idea 2) remain future work.
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

// ENABLE_TT_AGING: BACKLOG #42 idea 1 — date-based TT aging (Fruit/Toga
// design). The table persists across searches within a game (only `ucinewgame`
// clears it, #46), and the t22 replacement policy is depth-preferred only — so
// a deep entry stored early in a game squats in its slot forever (shallow
// stores can't evict: `depth >=` fails), crowding out fresh results. Fix: pack
// a 6-bit search date into the upper 6 bits of the node_type byte (bound types
// are 0..2 = 2 bits; entry stays 16 bytes). `new_search()` bumps the global
// date once per search (clearForSearch); a store may then evict any entry
// whose date isn't current, regardless of depth; a probe hit refreshes the
// entry's date so hot entries stay alive. Aging changes only WHICH entries
// survive, never the correctness of a returned entry (bound logic untouched;
// probe still masks the type, so callers are unchanged). SHIPPED baseline-t34
// (default ON) on the LTC verdict — blitz was inconclusive (t23 queue: AMD
// flat / Intel +11 lean) but the 60+0.6 leg confirmed aging pays where
// staleness accumulates (+15.99 ± 17.00, LOS 96.77%, 500g). Build the
// pre-t34 arm with -DENABLE_TT_AGING=0 (byte-identical baseline behavior).
#ifndef ENABLE_TT_AGING
#define ENABLE_TT_AGING 1  // shipped t34 (LTC leg vs t33, +15.99, LOS 96.77%)
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
    uint8_t node_type;       ///< Bits 0-1: EXACT/LOWER/UPPER bound; bits 2-7: search date (#42, ENABLE_TT_AGING)
    uint32_t best_move;      ///< Best move found during search (encoded S_MOVE format)

    /// Node types for alpha-beta bounds management
    static constexpr uint8_t EXACT = 0;        ///< Exact score within alpha-beta window
    static constexpr uint8_t LOWER_BOUND = 1;  ///< Beta cutoff (score >= beta)
    static constexpr uint8_t UPPER_BOUND = 2;  ///< Alpha never improved (score <= alpha)

    /// Bound type occupies the low 2 bits of node_type; the age lives above it.
    static constexpr uint8_t TYPE_MASK = 0x3;

    /// Bound type (EXACT/LOWER_BOUND/UPPER_BOUND) — always read node_type
    /// through this so the #42 age bits never leak into bound comparisons.
    uint8_t get_type() const { return node_type & TYPE_MASK; }
    /// Search date this entry was stored/refreshed under (6-bit, #42).
    uint8_t get_age() const { return static_cast<uint8_t>(node_type >> 2); }
    /// Re-date the entry without touching its bound type (#42 probe touch).
    void set_age(uint8_t age) {
        node_type = static_cast<uint8_t>((node_type & TYPE_MASK) | (age << 2));
    }

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
    // mutable: probe() is logically a read but re-dates the hit entry under
    // ENABLE_TT_AGING (#42 touch) — same const-method bookkeeping class as the
    // mutable hits/misses counters below.
    mutable std::vector<TTEntry> table;    ///< Main hash table storage
    size_t size_mask;              ///< Bit mask for fast modulo (table.size() - 1)

    // #42: global search date, bumped by new_search() once per search. 6-bit
    // (wraps at 64) to fit the upper bits of TTEntry::node_type. Unused
    // (stays 0) when ENABLE_TT_AGING is 0.
    static constexpr uint8_t AGE_MASK = 0x3F;
    uint8_t current_age = 0;

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
        current_age = 0;  // #42: fresh table, fresh date
        hits = misses = writes = 0;
    }

    // #42 (ENABLE_TT_AGING): advance the global search date. Called once per
    // search (Engine::clearForSearch); entries stored under an older date
    // become evictable by ANY store regardless of depth. 6-bit wraparound —
    // after 64 searches an untouched survivor's date reads current again and
    // depth-preferred replacement re-applies until the next bump (acceptable:
    // a slot that survives 64 searches untouched is vanishingly rare).
    // Compiles to a no-op when the flag is off, so the t22 arm's replacement
    // policy is untouched.
    void new_search() {
#if ENABLE_TT_AGING
        current_age = (current_age + 1) & AGE_MASK;
#endif
    }

    // #42: current search date (test hook; 0 when ENABLE_TT_AGING is 0).
    uint8_t get_current_age() const { return current_age; }
    
    /**
     * @brief Stores a search result, using depth-preferred replacement.
     * @param zobrist_key Position hash (also the verification key).
     * @param score Score to store (mate-distance-adjusted by the caller).
     * @param depth Search depth that produced the score.
     * @param node_type ::TTEntry EXACT / LOWER_BOUND / UPPER_BOUND.
     * @param best_move Best move found, for ordering future probes.
     *
     * Replaces the slot when it is empty, holds the same position (refresh), or
     * the incoming search is at least as deep. Under ENABLE_TT_AGING (#42) a
     * stale-dated resident (stored under an earlier search) is also replaced
     * regardless of depth; without aging, a strictly-shallower store into a
     * different deep entry is dropped and that deep entry is never evicted by
     * depth alone.
     */
    void store(uint64_t zobrist_key, int score, uint8_t depth, uint8_t node_type, uint32_t best_move = 0) {
        size_t index = zobrist_key & size_mask;
        TTEntry& entry = table[index];

        // Depth-preferred replacement: empty | same position | deeper-or-equal
        // (#42 aging arm adds: | stale date).
        bool should_replace = false;

        if (entry.zobrist_key == 0) {
            should_replace = true;          // empty slot
        } else if (entry.zobrist_key == zobrist_key) {
            should_replace = true;          // same position — refresh with newer info
#if ENABLE_TT_AGING
        } else if (entry.get_age() != current_age) {
            should_replace = true;          // stale resident from an earlier search (#42)
#endif
        } else if (depth >= entry.depth) {
            should_replace = true;          // collision, but ours is at least as deep
        } else {
            should_replace = false;         // keep the deeper current-date resident
        }

        if (should_replace) {
            entry.zobrist_key = zobrist_key;
            entry.score = score;
            entry.depth = depth;
#if ENABLE_TT_AGING
            // Pack the current date above the 2-bit bound type (entry stays 16B).
            entry.node_type = static_cast<uint8_t>((node_type & TTEntry::TYPE_MASK)
                                                   | (current_age << 2));
#else
            entry.node_type = node_type;
#endif
            entry.best_move = best_move;
#if ENABLE_INFO_DIAGNOSTICS
            writes++;  // Track write operations
#endif
        }
    }

    /**
     * @brief Looks up a position; on a key match, returns its stored result.
     * @param zobrist_key Position hash to look up.
     * @param[out] score Stored score (caller un-adjusts mate distance).
     * @param[out] depth Stored search depth.
     * @param[out] node_type Stored bound type (EXACT / LOWER / UPPER).
     * @param[out] best_move Stored best move (for ordering).
     * @return true on a full-key match (hit); false otherwise (out-params untouched).
     */
    bool probe(uint64_t zobrist_key, int& score, uint8_t& depth, uint8_t& node_type, uint32_t& best_move) const {
        size_t index = zobrist_key & size_mask;
        TTEntry& entry = table[index];

        if (entry.zobrist_key == zobrist_key) {
            score = entry.score;
            depth = entry.depth;
            // Masked read: callers always see EXACT/LOWER/UPPER (0..2), never
            // the #42 age bits (which are 0 anyway when the flag is off).
            node_type = entry.get_type();
            best_move = entry.best_move;
#if ENABLE_TT_AGING
            entry.set_age(current_age);  // touch: keep hot entries current (#42)
#endif
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
    
    // Clear all entries (ucinewgame, #46)
    void clear() {
        for (auto& entry : table) {
            entry = TTEntry();
        }
        current_age = 0;  // #42: new game restarts the search-date clock
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
