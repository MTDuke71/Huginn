/**
 * @file search_pool.hpp
 * @brief Lazy SMP thread pool — the shared-resource owner and search driver (#40 phase 3).
 *
 * Lazy SMP runs N independent searches over the same root, sharing nothing but
 * a transposition table. There is no work splitting and no synchronisation
 * inside the tree: threads desynchronise (here via depth skew), explore
 * differently-shaped trees, and cross-pollinate purely through TT entries.
 *
 * ## Ownership
 *
 * ::SearchPool owns the two resources every worker shares — the transposition
 * table and the opening book — and hands them to each worker's `Huginn::Engine`
 * through the phase-2 sharing constructor. Everything else an Engine owns
 * (history, killers, counter-moves, PV table, node counter, stop flag) stays
 * private to its thread, which is exactly what Lazy SMP wants.
 *
 * The pool persists across searches, so the TT survives between moves (only
 * `ucinewgame` clears it, #46) and worker Engines are reused rather than
 * rebuilt per move.
 *
 * ## Division of labour
 *
 * Thread 0 is the **driver**: it does the once-per-search setup (book and
 * tablebase root probes, the global TT date bump), it is the only thread that
 * writes to stdout, and its result is the move that gets played. Threads 1..N-1
 * are **helpers**: no output, no stdin polling, no time gate — they run until
 * the group stop fires and contribute only through the shared TT.
 *
 * Helpers contributing solely via the TT is deliberate for the first cut. The
 * alternative (pick the best result across threads by completed depth) is
 * usually worth a little Elo but adds a cross-thread selection race and makes a
 * bad move harder to attribute; it is queued as a follow-up once plain Lazy SMP
 * is measured and stable.
 *
 * @see docs/SMP_PHASE3_DESIGN.md for the full design and its rationale.
 * @see search.hpp for Engine's sharing constructor and the per-thread SearchInfo flags.
 */
#pragma once

#include "search.hpp"
#include "polyglot_book.hpp"
#include "syzygy_tablebase.hpp"
#include "transposition_table.hpp"

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

namespace Huginn {

/**
 * @brief Owns the shared search resources and runs an N-thread Lazy SMP search.
 *
 * Construct once (per engine instance), resize with set_threads(), then call
 * search() per move. With `threads == 1` no thread is ever spawned and the
 * call path is exactly the single-threaded one — that equivalence is what the
 * `Threads=1` bit-identity gate rests on.
 */
class SearchPool {
public:
    /// Upper bound on workers. Generous; the practical limit is core count.
    static constexpr int MAX_THREADS = 64;

    explicit SearchPool(SyzygyTablebase* tb = nullptr)
        : tablebase_(tb) {
        set_threads(1);
    }

    // Non-copyable, non-movable: workers hold references into this object.
    SearchPool(const SearchPool&) = delete;
    SearchPool& operator=(const SearchPool&) = delete;

    /**
     * @brief Sets the worker count, rebuilding the Engine set.
     *
     * Must not be called while a search is running — all workers are joined
     * before search() returns, so any call from the UCI command loop is safe.
     * Clamped to [1, MAX_THREADS]. Rebuilding drops each worker's per-thread
     * ordering tables (history/killers/counters) but NOT the shared TT, which
     * outlives any thread-count change.
     */
    void set_threads(int n);

    /// Current worker count (1 = single-threaded, no threads spawned).
    int threads() const { return static_cast<int>(workers_.size()); }

    /**
     * @brief Runs one search and returns the move to play.
     *
     * Does the once-per-search setup itself (book probe, tablebase root probe,
     * the global TT date bump via clearForSearch on the driver), spawns
     * `threads()-1` helpers, runs the driver's own iterative deepening on the
     * calling thread, then releases and joins the helpers.
     *
     * @param pos  Root position. Searched in place by the driver; helpers get copies.
     * @param info The driver's search state — limits in, statistics and PV out.
     *             `on_input` (the #56 UCI pump) stays installed on this one only.
     * @return The best move (the driver's result).
     */
    S_MOVE search(Position& pos, SearchInfo& info);

    /// Signals every worker to stop, from any thread (#56 `stop`, or a test).
    void stop_all();

    /// #46 `ucinewgame`: clears the shared TT and every worker's ordering
    /// tables, so no cross-game state survives into the next game.
    void new_game();

    /// Sum of every worker's published node count for the current search.
    uint64_t total_nodes() const;

    /// The shared table. Callers use this for the UCI `Hash` option and #46 clears.
    TranspositionTable& tt() { return tt_; }
    /// The shared opening book (`OwnBook` / `BookFile`).
    PolyglotBook& book() { return book_; }

    /// Rebinds the tablebase handle on every worker (UCI `SyzygyPath`).
    void set_tablebase(SyzygyTablebase* tb);

    /// The driver Engine — the single-threaded engine for tests, bench, and
    /// any caller that wants direct access (eval, move_to_uci, search tables).
    Engine& driver() { return *workers_[0]; }

private:
    TranspositionTable tt_{64};
    PolyglotBook book_;
    SyzygyTablebase* tablebase_;

    std::vector<std::unique_ptr<Engine>> workers_;

    /// The group cancellation channel every worker's checkup() consults.
    std::atomic<bool> group_stop_{false};

    /// Per-worker node mirrors, published from checkup() at ~2048-node cadence.
    /// Heap-allocated because std::atomic is not movable, so a vector of them
    /// cannot be resized.
    std::unique_ptr<std::atomic<uint64_t>[]> node_counts_;
};

}  // namespace Huginn
