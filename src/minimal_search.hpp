#pragma once

#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "pvtable.hpp"
#include <chrono>

namespace Huginn {

struct MinimalLimits {
    int max_depth = 6;
    int max_time_ms = 5000;
    bool infinite = false;
};

class MinimalEngine {
public:  // Make members public for easier access
    bool should_stop = false;
    int nodes_searched = 0;
    std::chrono::steady_clock::time_point start_time;
    MinimalLimits current_limits;
    PVTable pv_table;  // Principal Variation table (VICE tutorial style)
    
    // Constructor
    MinimalEngine() : pv_table(2) {}  // 2MB default size like tutorial
    
    // Simple material evaluation
    int evaluate(const Position& pos);
    
    // Basic alpha-beta search
    int alpha_beta(Position& pos, int depth, int alpha, int beta);
    
    // Time checking
    bool time_up() const;
    
    S_MOVE search(Position pos, const MinimalLimits& limits);
    void stop() { should_stop = true; }
    void reset() { should_stop = false; nodes_searched = 0; }
    
    // Utility to convert move to UCI string
    static std::string move_to_uci(const S_MOVE& move);
    
    // Simple repetition detection (VICE tutorial style)
    static bool isRepetition(const Position& pos);
    
    // PV table helper functions
    void store_pv_move(uint64_t position_key, const S_MOVE& move);
    bool probe_pv_move(uint64_t position_key, S_MOVE& move) const;
    
    // Get PV line for display (Part 53)
    int get_pv_line(Position& pos, int depth, S_MOVE pv_array[MAX_DEPTH]);
};

} // namespace Huginn
