#pragma once

#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "evaluation.hpp"
#include <string>
#include <vector>
#include <chrono>

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
    uint64_t max_time_ms = 5000;
    uint64_t max_nodes = 1000000;
    bool infinite = false;
};

// Simple single-threaded search engine
class SimpleEngine {
private:
    SearchStats stats;
    PVLine main_pv;
    std::chrono::steady_clock::time_point start_time;
    SearchLimits current_limits;
    bool should_stop = false;
    
    // Core search functions
    int alpha_beta(Position& pos, int depth, int alpha, int beta, PVLine& pv);
    int quiescence_search(Position& pos, int alpha, int beta);
    
    // Move ordering
    void order_moves(const Position& pos, S_MOVELIST& moves);
    int score_move(const Position& pos, const S_MOVE& move);
    
    // Time management
    bool time_up() const;
    void update_stats();
    
public:
    SimpleEngine() = default;
    
    // Main search interface
    S_MOVE search(Position pos, const SearchLimits& limits);
    
    // Information access
    const SearchStats& get_stats() const { return stats; }
    const PVLine& get_pv() const { return main_pv; }
    
    // Control
    void stop() { should_stop = true; }
    void reset() { 
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

} // namespace Huginn
