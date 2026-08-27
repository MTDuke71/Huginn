/**
 * @file search_pool.cpp
 * @brief Lazy SMP thread pool implementation (#40 phase 3).
 * @see search_pool.hpp for the design contract.
 */
#include "search_pool.hpp"

#include "movegen.hpp"

#include <algorithm>
#include <iostream>

namespace Huginn {

void SearchPool::set_threads(int n) {
    n = std::max(1, std::min(n, MAX_THREADS));
    const int cur = static_cast<int>(workers_.size());
    if (cur == n) return;

    // GROW/SHRINK in place rather than rebuilding. Worker 0 must survive a
    // thread-count change: driver() hands out a reference to it, and the UCI
    // layer caches that as `search_engine`. Rebuilding the vector would leave
    // that dangling the moment a GUI sent `setoption name Threads`. Keeping
    // existing workers also preserves their ordering tables, so changing
    // Threads mid-game doesn't wipe the driver's history.
    //
    // Note the vector's own buffer may reallocate on push_back — that moves the
    // unique_ptrs, not the Engines, so Engine addresses (and driver()) stay
    // stable regardless.
    if (n < cur) {
        workers_.resize(static_cast<size_t>(n));
    } else {
        workers_.reserve(static_cast<size_t>(n));
        for (int i = cur; i < n; ++i) {
            // Every worker binds to the SAME table and book (the phase-2
            // sharing constructor) while keeping its own ordering tables.
            workers_.push_back(std::make_unique<Engine>(tt_, book_, tablebase_));
        }
    }

    node_counts_ = std::make_unique<std::atomic<uint64_t>[]>(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) node_counts_[i].store(0, std::memory_order_relaxed);
}

void SearchPool::set_tablebase(SyzygyTablebase* tb) {
    tablebase_ = tb;
    for (auto& w : workers_) w->tablebase = tb;
}

void SearchPool::stop_all() {
    group_stop_.store(true, std::memory_order_relaxed);
    // Also raise each Engine's own flag, so a worker that is between checkup()
    // calls on the group flag still sees a stop through the #56 path.
    for (auto& w : workers_) w->stop();
}

void SearchPool::new_game() {
    // The TT is shared, so one clear covers every worker (#46). Ordering
    // tables are per-worker and each needs its own.
    tt_.clear();
    for (auto& w : workers_) {
        w->reset();
        w->clear_search_tables();
        w->pv_table.clear();
    }
}

uint64_t SearchPool::total_nodes() const {
    uint64_t sum = 0;
    for (int i = 0; i < static_cast<int>(workers_.size()); ++i) {
        sum += node_counts_[i].load(std::memory_order_relaxed);
    }
    return sum;
}

S_MOVE SearchPool::search(Position& pos, SearchInfo& info) {
    Engine& driver = *workers_[0];
    const int n = static_cast<int>(workers_.size());

    // ---- Arm ----------------------------------------------------------------
    // Flag hygiene runs FIRST, before the probes below can short-circuit: a
    // stray `stop` that arrived between searches must not leak into this one.
    // (#73: this used to be split between UCI's pre-search reset() and stores
    // scattered through this function — a book-hit return skipping the clears
    // was one missed store away from releasing a `go infinite` early.)
    group_stop_.store(false, std::memory_order_relaxed);
    for (auto& w : workers_) w->stop_token.clear();
    for (int i = 0; i < n; ++i) node_counts_[i].store(0, std::memory_order_relaxed);

    driver.stop_token.attach_group(&group_stop_);
    // Pin the driver role regardless of what the caller set: thread 0, sole
    // stdout writer, owns the time policy, hosts the #56 UCI pump. Before the
    // probes: root_shortcut's announcements obey emit_uci_output.
    info.thread_id = 0;
    info.emit_uci_output = true;
    info.use_iteration_gate = true;
    info.polls_input = true;
    info.node_mirror = &node_counts_[0];
    info.aggregate_nodes = [this]() { return total_nodes(); };

    // ---- Once-per-search setup (driver only) --------------------------------
    // Book and tablebase root probes short-circuit the ENTIRE search, so they
    // run before anything is spawned. ONE copy, shared with searchPosition.
    S_MOVE shortcut = driver.root_shortcut(pos, info);
    if (shortcut.move != 0) return shortcut;

    // clearForSearch bumps the GLOBAL TT search date (#42), so it runs exactly
    // ONCE here — never per worker. Helpers get their own per-thread table
    // clear below instead.
    Engine::clearForSearch(driver, info);

    // ---- Spawn helpers -------------------------------------------------------
    std::vector<std::thread> helpers;
    std::vector<SearchInfo> helper_info(static_cast<size_t>(std::max(0, n - 1)));
    std::vector<Position> helper_pos(static_cast<size_t>(std::max(0, n - 1)), pos);

    helpers.reserve(static_cast<size_t>(std::max(0, n - 1)));
    for (int i = 1; i < n; ++i) {
        Engine& w = *workers_[i];
        SearchInfo& hi = helper_info[static_cast<size_t>(i - 1)];

        // Per-thread ordering tables only — NOT clearForSearch, which would
        // re-bump the shared TT date. Stop tokens were already cleared in the
        // arm step above.
        w.clear_search_tables();
        w.pv_table.clear();
        w.stop_token.attach_group(&group_stop_);

        // The helper's SearchInfo contract — which of the driver's fields are
        // inherited (and why soft_stop_time is not) — lives in the factory.
        hi = SearchInfo::for_helper(info, i, &node_counts_[i]);

        helpers.emplace_back([&w, &hi, &helper_pos, i]() {
            w.iterativeDeepening(helper_pos[static_cast<size_t>(i - 1)], hi);
        });
    }

    // ---- Driver searches on this thread --------------------------------------
    // The join below is NOT optional on the throwing path: a live std::thread
    // whose destructor runs unjoined calls std::terminate, which would turn a
    // recoverable search exception (the UCI layer catches those and falls back
    // to a legal move) into a process kill. So release + join happen either way
    // and the exception is rethrown afterwards.
    S_MOVE best;
    try {
        best = driver.iterativeDeepening(pos, info);
    } catch (...) {
        release_and_join(helpers);
        throw;
    }

    // The driver finishing (on time, depth, or `stop`) ends the whole search.
    release_and_join(helpers);
    return best;
}

/// #73: the one exit path — trip the group stop, fan out to every worker's
/// token (a worker between checkup() polls on the group flag still sees the
/// stop through its local channel), join the helpers, then leave every flag
/// clean for the next search / for single-threaded callers.
void SearchPool::release_and_join(std::vector<std::thread>& helpers) {
    group_stop_.store(true, std::memory_order_relaxed);
    for (auto& w : workers_) w->stop();
    for (auto& t : helpers) t.join();

    group_stop_.store(false, std::memory_order_relaxed);
    for (auto& w : workers_) w->stop_token.clear();
}

}  // namespace Huginn
