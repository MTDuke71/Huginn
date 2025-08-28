#pragma once

#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "evaluation.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>

#ifdef _MSC_VER
#include "msvc_optimizations.hpp"
#endif

namespace Huginn {

// Search constants
constexpr int MAX_PLY = 100;
constexpr int MATE_SCORE = 32000;
constexpr int MATE_IN_MAX_PLY = MATE_SCORE - MAX_PLY;

// Simple search statistics
struct SearchStats {
    uint64_t nodes_searched = 0;
    uint64_t time_ms = 0;
    int max_depth_reached = 0;
    
    void reset() {
        nodes_searched = 0;
        time_ms = 0;
        max_depth_reached = 0;
    }
};

// Principal Variation line
struct PVLine {
    S_MOVE moves[64];
    int length = 0;
    
    void clear() { length = 0; }
    void add_move(S_MOVE move) {
        if (length < 63) {
            moves[length++] = move;
        }
    }
};

// Simple search limits
struct SearchLimits {
    int max_depth = 0;
    uint64_t max_time_ms = 10000;  // 10 seconds default
    uint64_t max_nodes = UINT64_MAX; // Effectively unlimited nodes
    bool infinite = false;
    int threads = 16; // Default to 16 threads for optimal performance
};

// Simple single-threaded search engine
class SimpleEngine {
protected:
    SearchStats stats;
    PVLine main_pv;
    std::chrono::steady_clock::time_point start_time;
    SearchLimits current_limits;
    bool should_stop = false;
    
    // Core search functions
    int alpha_beta(Position& pos, int depth, int alpha, int beta, PVLine& pv);
    int quiescence_search(Position& pos, int alpha, int beta);
    
    // Node counting (virtual for thread-safe override)
    virtual void increment_nodes() { stats.nodes_searched++; }
    
    // Move ordering
    void order_moves(const Position& pos, S_MOVELIST& moves);
    int score_move(const Position& pos, const S_MOVE& move);
    
    // Time management
    bool time_up() const;
    void update_stats();

private:
    
public:
    SimpleEngine() = default;
    virtual ~SimpleEngine() = default;
    
    // Main search interface
    virtual S_MOVE search(Position pos, const SearchLimits& limits);
    
    // Information access
    virtual const SearchStats& get_stats() const { return stats; }
    const PVLine& get_pv() const { return main_pv; }
    
    // Control
    virtual void stop() { should_stop = true; }
    virtual void reset() { 
        stats.reset(); 
        main_pv.clear(); 
        should_stop = false; 
    }
    
    // Utility functions
    static std::string move_to_uci(const S_MOVE& move);
    static std::string pv_to_string(const PVLine& pv);
    
    // Mate detection
    static bool is_mate_score(int score) { return std::abs(score) > MATE_IN_MAX_PLY; }
    static int mate_distance(int score);
    static std::string score_to_uci(int score);
};

// Thread-safe search statistics
struct ThreadSafeStats {
    std::atomic<uint64_t> nodes_searched{0};
    std::atomic<uint64_t> time_ms{0};
    std::atomic<int> max_depth_reached{0};
    
    void reset() {
        nodes_searched = 0;
        time_ms = 0;
        max_depth_reached = 0;
    }
    
    SearchStats to_search_stats() const {
        SearchStats stats;
        stats.nodes_searched = nodes_searched.load();
        stats.time_ms = time_ms.load();
        stats.max_depth_reached = max_depth_reached.load();
        return stats;
    }
};

// Multi-threaded search engine
class ThreadedEngine : public SimpleEngine {
private:
    ThreadSafeStats thread_safe_stats;
    std::atomic<bool> global_stop{false};
    std::mutex pv_mutex;
    std::mutex output_mutex;
    std::mutex best_move_mutex;
    
    // Shared best move across all threads
    S_MOVE global_best_move;
    PVLine global_best_pv;
    std::atomic<int> global_best_score{-MATE_SCORE};
    
    // Thread worker function
    S_MOVE thread_search_worker(Position pos, const SearchLimits& limits, int thread_id);
    
    // Thread-safe time checking
    bool thread_time_up() const;
    
    // Thread-safe node counting
    void increment_nodes() override { thread_safe_stats.nodes_searched++; }
    
    // Single-threaded fallback
    S_MOVE single_threaded_search(Position pos, const SearchLimits& limits);
    
public:
    ThreadedEngine() = default;
    
    // Main threaded search interface
    S_MOVE search(Position pos, const SearchLimits& limits) override;
    
    // Thread-safe control
    void stop() override { 
        SimpleEngine::stop();
        global_stop = true; 
    }
    
    void reset() override { 
        SimpleEngine::reset();
        thread_safe_stats.reset();
        global_stop = false; 
    }
    
    // Thread-safe information access
    const SearchStats& get_stats() const override {
        static SearchStats cached_stats;
        cached_stats = thread_safe_stats.to_search_stats();
        return cached_stats;
    }
};

} // namespace Huginn
