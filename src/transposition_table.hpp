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
 * - **One entry per index** on the baseline arm: a store collides directly with
 *   whatever shares its index. Verification stores the *full* 64-bit Zobrist key
 *   and compares it on probe, so a wrong-position hit is effectively impossible
 *   (no separate index/lock split). ENABLE_TT_CLUSTERS (#42b) changes the index
 *   unit to a 4-entry cluster — see the flag block below.
 * - 16-byte cells (8 key + 2 score + 1 depth + 1 type + 4 move) for cache-line
 *   friendliness (4 cells = one 64-byte line, the #42b cluster).
 * - **Lockless-safe storage** (#40 phase 1): the two 8-byte halves are relaxed
 *   atomics and the key half is stored XOR'd with the payload, so a read torn
 *   across a concurrent store decodes to a bogus key and is reported as a miss.
 *   See ::TTSlot. Single-threaded behaviour is unchanged (byte-identical to
 *   baseline-t34).
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
 * re-date the entry. ENABLE_TT_CLUSTERS (#42 idea 2, "#42b") makes the table
 * 4-way set-associative so colliding hot positions stop thrashing one slot.
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

// ENABLE_TT_CLUSTERS: BACKLOG #42 idea 2 ("#42b") — N-way clusters (Fruit/Toga
// design), the queued follow-up to the t34 aging ship. The baseline table is
// direct-mapped (one entry per index), so two hot positions sharing an index
// evict each other every time regardless of policy — the only tiebreak is who
// wrote last with enough depth. Fix: the index unit becomes a 4-entry cluster
// (4 × 16B = one 64-byte cache line, alignas(64), so the whole cluster costs
// the same memory traffic as one baseline probe). Probe scans the cluster for
// a full-key match; store refreshes a same-key slot, else fills an empty slot,
// else replaces the least-valuable resident by Fruit's replaceability value
// `age_dist*256 - depth` (stale-dated first, then shallowest — composes with
// ENABLE_TT_AGING; with aging off the formula collapses to min-depth).
// REVISION 2: r1 was Fruit-faithful always-store (a full current-date cluster
// never dropped an incoming shallower store) and measured NEGATIVE at blitz
// (AMD −9.38 ± 14.81, LOS 10.69%, 1000g vs t34) — every depth-1/2 store
// churned the cluster's 4th-deepest entry, washing out mid-depth results.
// r2 restores the baseline drop gate against the WEAKEST resident: a
// current-date victim is only displaced by an at-least-as-deep store. That
// still drops strictly fewer stores than the baseline arm (weakest-of-4 ≤
// the one random slot occupant) while never evicting deep work for junk.
// Same total entry count at any Hash size (the index just loses 2 bits).
// Candidate, default OFF — flag-off is byte-identical to baseline-t34.
#ifndef ENABLE_TT_CLUSTERS
#define ENABLE_TT_CLUSTERS 0  // candidate #42b (default OFF)
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
 * @struct TTSlot
 * @brief Lockless-safe storage cell holding one ::TTEntry (#40 phase 1).
 *
 * Under Lazy SMP every worker probes and stores into the SAME table with no
 * lock, so a 16-byte entry can be read while another thread is halfway through
 * writing it — a **torn** read whose key comes from one store and whose payload
 * comes from another. That is not merely a stale entry: `best_move` would be a
 * move from an unrelated position, handed straight to `pick_next_move` for
 * ordering (and, via the TT-move path, to singular extensions).
 *
 * Hyatt's XOR trick makes tearing self-detecting at zero cost. The cell stores
 * `key_xor = zobrist_key ^ data` next to `data`; a reader recovers the key as
 * `key_xor ^ data`, which equals the probed key only when both halves came from
 * the *same* store. Any mismatched pair decodes to a key that (barring a 2^-64
 * coincidence) fails the probe's full-key compare and is reported as a miss —
 * exactly the outcome a slot-eviction race should produce anyway. This also
 * makes good on the guarantee the header above already claimed: a wrong-position
 * hit is effectively impossible.
 *
 * Both halves are `relaxed` atomics. On x86-64 that emits the same plain 64-bit
 * `mov` the non-atomic fields did — the fixed-depth signature is unchanged —
 * but it makes the concurrent access defined behaviour instead of a data race.
 * No ordering is needed between the two: every torn combination is rejected.
 *
 * An empty cell is `{0, 0}`, which decodes to key 0 — the same "key 0 means
 * empty" convention the direct-mapped arm has always used.
 */
struct TTSlot {
    std::atomic<uint64_t> key_xor;  ///< zobrist_key ^ data (0 when empty)
    std::atomic<uint64_t> data;     ///< score | depth | node_type | best_move

    /// Payload packing: score in bits 0-15, depth 16-23, node_type 24-31,
    /// best_move 32-63. Exactly the 8 non-key bytes of a ::TTEntry.
    static uint64_t pack(const TTEntry& e) {
        return uint64_t(static_cast<uint16_t>(e.score))
             | (uint64_t(e.depth) << 16)
             | (uint64_t(e.node_type) << 24)
             | (uint64_t(e.best_move) << 32);
    }
    static void unpack(uint64_t d, TTEntry& e) {
        e.score     = static_cast<int16_t>(static_cast<uint16_t>(d & 0xFFFFu));
        e.depth     = static_cast<uint8_t>((d >> 16) & 0xFFu);
        e.node_type = static_cast<uint8_t>((d >> 24) & 0xFFu);
        e.best_move = static_cast<uint32_t>(d >> 32);
    }

    TTSlot() : key_xor(0), data(0) {}

    // Copyable so the table can live in a std::vector (std::atomic is neither
    // copyable nor movable on its own). Only ever used for bulk (re)init in
    // resize_mb — never while a search is running.
    TTSlot(const TTSlot& o)
        : key_xor(o.key_xor.load(std::memory_order_relaxed)),
          data(o.data.load(std::memory_order_relaxed)) {}
    TTSlot& operator=(const TTSlot& o) {
        key_xor.store(o.key_xor.load(std::memory_order_relaxed), std::memory_order_relaxed);
        data.store(o.data.load(std::memory_order_relaxed), std::memory_order_relaxed);
        return *this;
    }

    /**
     * @brief Decodes the cell into `out` and returns the key it claims.
     * @return The recovered Zobrist key: 0 for an empty cell, the stored key for
     *         a clean read, and effectively-random garbage for a torn read (which
     *         the caller's full-key compare then rejects as a miss).
     */
    uint64_t load(TTEntry& out) const {
        const uint64_t d = data.load(std::memory_order_relaxed);
        const uint64_t k = key_xor.load(std::memory_order_relaxed);
        unpack(d, out);
        return k ^ d;
    }

    /// Key-only decode, for callers that don't need the payload (`permill_full`).
    uint64_t load_key() const {
        return key_xor.load(std::memory_order_relaxed)
             ^ data.load(std::memory_order_relaxed);
    }

    /// Publishes `e` under `key`. Payload first, then the XOR'd key — though the
    /// order is immaterial, since both torn combinations decode to a bad key.
    void store_entry(uint64_t key, const TTEntry& e) {
        const uint64_t d = pack(e);
        data.store(d, std::memory_order_relaxed);
        key_xor.store(key ^ d, std::memory_order_relaxed);
    }

    /// Returns the cell to the empty state (`ucinewgame` / resize).
    void reset() {
        key_xor.store(0, std::memory_order_relaxed);
        data.store(0, std::memory_order_relaxed);
    }
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
#if ENABLE_TT_CLUSTERS
    /// #42b: the index unit — 4 × 16B cells = exactly one 64-byte cache
    /// line, so a whole-cluster probe scan costs the same memory traffic as a
    /// single-entry probe on the baseline arm.
    static constexpr size_t CLUSTER_SIZE = 4;
    struct alignas(64) TTCluster { TTSlot e[CLUSTER_SIZE]; };
#endif

    // mutable: probe() is logically a read but re-dates the hit entry under
    // ENABLE_TT_AGING (#42 touch) — same const-method bookkeeping class as the
    // mutable hits/misses counters below.
#if ENABLE_TT_CLUSTERS
    mutable std::vector<TTCluster> table;  ///< Main hash table storage (#42b: cluster-indexed)
    size_t size_mask;              ///< Bit mask for fast modulo (cluster count - 1)
#else
    mutable std::vector<TTSlot> table;     ///< Main hash table storage
    size_t size_mask;              ///< Bit mask for fast modulo (table.size() - 1)
#endif

    // #42: global search date, bumped by new_search() once per search. 6-bit
    // (wraps at 64) to fit the upper bits of TTEntry::node_type. Unused
    // (stays 0) when ENABLE_TT_AGING is 0.
    static constexpr uint8_t AGE_MASK = 0x3F;
    uint8_t current_age = 0;

    /// Builds the payload a store publishes, stamping the current search date
    /// above the 2-bit bound type (#42). Shared by both replacement arms so the
    /// date-packing rule has exactly one definition.
    TTEntry make_entry(int score, uint8_t depth, uint8_t node_type, uint32_t best_move) const {
        TTEntry e;
        e.score = static_cast<int16_t>(score);
        e.depth = depth;
#if ENABLE_TT_AGING
        e.node_type = static_cast<uint8_t>((node_type & TTEntry::TYPE_MASK)
                                           | (current_age << 2));
#else
        e.node_type = node_type;
#endif
        e.best_move = best_move;
        return e;
    }

#if ENABLE_TT_AGING
    /// #42 probe touch: re-date a hit so hot entries stay current.
    ///
    /// Skipped when the date already matches. Single-threaded that is exactly
    /// the old behaviour (the unconditional `set_age` rewrote the byte it
    /// already held); under SMP it turns the overwhelmingly common case from a
    /// gratuitous store — which invalidates the cache line in every other
    /// worker — into a pure read. When the date HAS moved, the touch republishes
    /// the whole cell; if that races a concurrent store to the same slot one of
    /// them simply wins. The table is a cache, so a lost store costs work, never
    /// correctness — and the XOR key guarantees no reader sees the blend.
    void touch(TTSlot& slot, uint64_t key, TTEntry& e) const {
        if (e.get_age() == current_age) return;
        e.set_age(current_age);
        slot.store_entry(key, e);
    }
#endif

    // Statistics tracking for performance analysis. Relaxed atomics (#40): with
    // ENABLE_INFO_DIAGNOSTICS on, every worker bumps these from its own thread —
    // plain uint64_t counters would be a data race (and would lose counts).
    // Relaxed is right: they are pure statistics, ordered against nothing.
    mutable std::atomic<uint64_t> hits{0};    // Successful probes
    mutable std::atomic<uint64_t> misses{0};  // Failed probes
    std::atomic<uint64_t> writes{0};          // Store operations

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

#if ENABLE_TT_CLUSTERS
        // #42b: round the CLUSTER count down to a power of 2 (minimum 1
        // cluster). Total entries match the baseline arm at any Hash size —
        // the index just addresses 4-entry lines instead of single entries.
        size_t num_clusters = num_entries / CLUSTER_SIZE;
        size_t power_of_2 = 1;
        while (power_of_2 * 2 <= num_clusters) {
            power_of_2 *= 2;
        }

        table.assign(power_of_2, TTCluster());  // resize + zero-initialize
        size_mask = power_of_2 - 1;
#else
        // Round down to nearest power of 2 for fast indexing (minimum 1 entry)
        size_t power_of_2 = 1;
        while (power_of_2 * 2 <= num_entries) {
            power_of_2 *= 2;
        }

        table.assign(power_of_2, TTSlot());  // resize + zero-initialize
        size_mask = power_of_2 - 1;
#endif
        current_age = 0;  // #42: fresh table, fresh date
        hits = 0; misses = 0; writes = 0;
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
     * depth alone. Under ENABLE_TT_CLUSTERS (#42b r2) the victim is same-key
     * slot > empty slot > least-valuable resident (stale-dated first, then
     * shallowest), and a current-date victim is only displaced by an
     * at-least-as-deep store (the baseline drop rule, applied to the weakest
     * resident — r1's always-store variant measured Elo-negative).
     */
    void store(uint64_t zobrist_key, int score, uint8_t depth, uint8_t node_type, uint32_t best_move = 0) {
#if ENABLE_TT_CLUSTERS
        TTSlot* const cluster = table[zobrist_key & size_mask].e;

        // Victim selection (#42b, Fruit): same key always refreshes in place;
        // otherwise the most replaceable slot by `age_dist*256 - depth` —
        // empty beats everything, then stale-dated (any age distance dwarfs
        // any depth), then shallowest. With ENABLE_TT_AGING off, ages are all
        // 0 and the value collapses to plain min-depth.
        // #40: each candidate is decoded once (a torn read just yields a bogus
        // key/depth, which at worst picks a different victim — the choice is a
        // heuristic, so a raced eviction is harmless).
        TTSlot* victim = nullptr;
        TTEntry victim_e;
        uint64_t victim_key = 0;
        int victim_value = -256;            // below any real slot's value
        for (size_t i = 0; i < CLUSTER_SIZE; ++i) {
            TTEntry e;
            const uint64_t k = cluster[i].load(e);
            if (k == zobrist_key) {
                victim = &cluster[i];       // same position — refresh with newer info
                victim_e = e;
                victim_key = k;
                break;
            }
            const int value = (k == 0)
                ? (1 << 30)                 // empty slot wins outright
                : static_cast<int>((current_age - e.get_age()) & AGE_MASK) * 256
                      - static_cast<int>(e.depth);
            if (value > victim_value) {
                victim_value = value;
                victim = &cluster[i];
                victim_e = e;
                victim_key = k;
            }
        }

        // r2 drop gate (the r1 park's diagnosis): a current-date victim is
        // only displaced by an at-least-as-deep store — the baseline arm's
        // drop rule, applied to the WEAKEST resident. r1's always-store let
        // every depth-1/2 store (the exponential bulk) churn the cluster's
        // 4th-deepest entry, washing out the mid-depth results that feed TT
        // cutoffs / IID / singular eligibility on re-visits (AMD blitz
        // −9.38, LOS 10.69%, 1000g). Same-key refreshes, empty slots, and
        // stale-dated victims still always land, so this drops strictly
        // FEWER stores than the baseline (weakest-of-4 ≤ the one random
        // occupant baseline compares against). With aging off, ages are all
        // 0 and the gate reduces to the baseline depth rule.
        if (victim_key != 0 && victim_key != zobrist_key
            && victim_e.get_age() == current_age
            && depth < victim_e.depth) {
            return;                         // keep the deeper current-date resident
        }

        victim->store_entry(zobrist_key, make_entry(score, depth, node_type, best_move));
#if ENABLE_INFO_DIAGNOSTICS
        writes.fetch_add(1, std::memory_order_relaxed);  // Track write operations
#endif
#else  // !ENABLE_TT_CLUSTERS — baseline direct-mapped arm
        TTSlot& slot = table[zobrist_key & size_mask];
        TTEntry resident;
        const uint64_t resident_key = slot.load(resident);

        // Depth-preferred replacement: empty | same position | deeper-or-equal
        // (#42 aging arm adds: | stale date).
        bool should_replace = false;

        if (resident_key == 0) {
            should_replace = true;          // empty slot
        } else if (resident_key == zobrist_key) {
            should_replace = true;          // same position — refresh with newer info
#if ENABLE_TT_AGING
        } else if (resident.get_age() != current_age) {
            should_replace = true;          // stale resident from an earlier search (#42)
#endif
        } else if (depth >= resident.depth) {
            should_replace = true;          // collision, but ours is at least as deep
        } else {
            should_replace = false;         // keep the deeper current-date resident
        }

        if (should_replace) {
            slot.store_entry(zobrist_key, make_entry(score, depth, node_type, best_move));
#if ENABLE_INFO_DIAGNOSTICS
            writes.fetch_add(1, std::memory_order_relaxed);  // Track write operations
#endif
        }
#endif  // ENABLE_TT_CLUSTERS
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
#if ENABLE_TT_CLUSTERS
        // #42b: scan the 4-slot cluster for a full-key match — all four
        // candidates live in the same 64-byte cache line as a baseline probe.
        TTSlot* const cluster = table[zobrist_key & size_mask].e;
        for (size_t i = 0; i < CLUSTER_SIZE; ++i) {
            TTEntry entry;
            if (cluster[i].load(entry) != zobrist_key) continue;
            score = entry.score;
            depth = entry.depth;
            // Masked read: callers always see EXACT/LOWER/UPPER (0..2), never
            // the #42 age bits (which are 0 anyway when the flag is off).
            node_type = entry.get_type();
            best_move = entry.best_move;
#if ENABLE_TT_AGING
            touch(cluster[i], zobrist_key, entry);  // keep hot entries current (#42)
#endif
#if ENABLE_INFO_DIAGNOSTICS
            hits.fetch_add(1, std::memory_order_relaxed);  // Track successful probes
#endif
            return true;
        }
#if ENABLE_INFO_DIAGNOSTICS
        misses.fetch_add(1, std::memory_order_relaxed);  // Track failed probes
#endif
        return false;
#else  // !ENABLE_TT_CLUSTERS — baseline direct-mapped arm
        TTSlot& slot = table[zobrist_key & size_mask];
        TTEntry entry;

        if (slot.load(entry) == zobrist_key) {
            score = entry.score;
            depth = entry.depth;
            // Masked read: callers always see EXACT/LOWER/UPPER (0..2), never
            // the #42 age bits (which are 0 anyway when the flag is off).
            node_type = entry.get_type();
            best_move = entry.best_move;
#if ENABLE_TT_AGING
            touch(slot, zobrist_key, entry);  // keep hot entries current (#42)
#endif
#if ENABLE_INFO_DIAGNOSTICS
            hits.fetch_add(1, std::memory_order_relaxed);  // Track successful probes
#endif
            return true;
        }
#if ENABLE_INFO_DIAGNOSTICS
        misses.fetch_add(1, std::memory_order_relaxed);  // Track failed probes
#endif
        return false;
#endif  // ENABLE_TT_CLUSTERS
    }

    // Clear all entries (ucinewgame, #46)
    void clear() {
#if ENABLE_TT_CLUSTERS
        for (auto& cluster : table) {
            for (auto& slot : cluster.e) slot.reset();
        }
#else
        for (auto& slot : table) {
            slot.reset();
        }
#endif
        current_age = 0;  // #42: new game restarts the search-date clock
        // Reset statistics
        hits = 0; misses = 0; writes = 0;
    }

    // Total ENTRY capacity on both arms (#42b: clusters × 4), so the UCI
    // "Hash resized" info line keeps reporting the same unit.
#if ENABLE_TT_CLUSTERS
    size_t get_size() const { return table.size() * CLUSTER_SIZE; }
#else
    size_t get_size() const { return table.size(); }
#endif

    // UCI standard `hashfull` — fraction of TT in use expressed in permill
    // (0..1000). Loop is O(N) but only called once per iterative-deepening
    // depth (one info line per depth), not per node. Always available
    // regardless of the diagnostics gate; UCI GUIs use this to colour
    // time-pressure indicators and decide adjudication.
    int permill_full() const {
#if ENABLE_TT_CLUSTERS
        const size_t n = table.size() * CLUSTER_SIZE;
#else
        const size_t n = table.size();
#endif
        if (n == 0) return 0;
        // Sample-based estimate to keep cost negligible on huge TTs (e.g. 256MB).
        // Walk the first 1000 entries; spec only requires accuracy to ~1‰.
        const size_t sample = n < 1000 ? n : 1000;
        size_t filled = 0;
        for (size_t i = 0; i < sample; ++i) {
#if ENABLE_TT_CLUSTERS
            if (table[i / CLUSTER_SIZE].e[i % CLUSTER_SIZE].load_key() != 0) ++filled;
#else
            if (table[i].load_key() != 0) ++filled;
#endif
        }
        return static_cast<int>((filled * 1000ULL) / sample);
    }

    // Get statistics
    uint64_t get_hits() const { return hits.load(std::memory_order_relaxed); }
    uint64_t get_writes() const { return writes.load(std::memory_order_relaxed); }
};
