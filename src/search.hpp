#pragma once

#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "evaluation.hpp"
#include <chrono>
#include <atomic>
#include <functional>

/**
 * Huginn Chess Engine - Search System
 * 
 * Implements minimax search with alpha-beta pruning, iterative deepening,
 * and various search enhancements for competitive play.
 */

namespace Search {

    // Search limits and constants
    constexpr int MAX_DEPTH = 64;
    constexpr int MAX_PLY = 100;
    constexpr int INFINITE_TIME = 1000000;
    
    // Search scores
    constexpr int MATE_SCORE = 32000;
    constexpr int MATE_IN_MAX_PLY = MATE_SCORE - MAX_PLY;
    
    // Search statistics
    struct SearchStats {
        uint64_t nodes_searched = 0;
        uint64_t qnodes_searched = 0;
        int depth_reached = 0;
        int selective_depth = 0;
        std::chrono::milliseconds time_elapsed{0};
        double nodes_per_second = 0.0;
        
        void reset() {
            nodes_searched = 0;
            qnodes_searched = 0;
            depth_reached = 0;
            selective_depth = 0;
            time_elapsed = std::chrono::milliseconds{0};
            nodes_per_second = 0.0;
        }
        
        void calculate_nps() {
            if (time_elapsed.count() > 0) {
                nodes_per_second = (nodes_searched * 1000.0) / time_elapsed.count();
            }
        }
    };

    // Search limits
    struct SearchLimits {
        int max_depth = MAX_DEPTH;
        std::chrono::milliseconds max_time{INFINITE_TIME};
        uint64_t max_nodes = UINT64_MAX;
        bool infinite = false;
        
        // Time control
        std::chrono::milliseconds remaining_time{0};
        std::chrono::milliseconds increment{0};
        int moves_to_go = 0;
        
        SearchLimits() = default;
    };

    // Search information for UCI
    struct SearchInfo {
        int depth = 0;
        int score = 0;
        uint64_t nodes = 0;
        int time_ms = 0;
        std::vector<S_MOVE> pv;
        
        SearchInfo() = default;
    };

    // Principal Variation (PV) line
    struct PVLine {
        S_MOVE moves[MAX_DEPTH];
        int length = 0;
        
        void clear() { length = 0; }
        
        void copy_from(const PVLine& other) {
            length = other.length;
            for (int i = 0; i < length; ++i) {
                moves[i] = other.moves[i];
            }
        }
        
        void add_move(const S_MOVE& move) {
            if (length < MAX_DEPTH) {
                moves[length++] = move;
            }
        }
        
        std::string to_string() const;
    };

    // Move ordering heuristics
    class MoveOrderer {
    private:
        // Killer moves (non-capture moves that caused beta cutoffs)
        S_MOVE killer_moves[MAX_PLY][2];
        
        // History heuristic (move ordering based on past success)
        int history_table[2][120][120]; // [color][from][to]
        
    public:
        MoveOrderer() { clear(); }
        
        void clear();
        void add_killer_move(int ply, const S_MOVE& move);
        void update_history(Color color, const S_MOVE& move, int depth);
        int get_move_score(const S_MOVE& move, Color color, int ply) const;
        void order_moves(S_MOVELIST& moves, Color color, int ply, const S_MOVE& pv_move = S_MOVE()) const;
    };

    // Transposition table entry
    enum class TTFlag : uint8_t {
        EXACT = 0,    // Exact score
        ALPHA = 1,    // Upper bound (failed low)
        BETA = 2      // Lower bound (failed high)
    };

    struct TTEntry {
        uint64_t key = 0;           // Position hash key
        S_MOVE best_move;           // Best move found
        int16_t score = 0;          // Position score
        int16_t eval = 0;           // Static evaluation
        uint8_t depth = 0;          // Search depth
        TTFlag flag = TTFlag::EXACT; // Score type
        uint8_t age = 0;            // Search generation
        
        bool is_valid(uint64_t pos_key) const {
            return key == pos_key;
        }
    };

    // Transposition table
    class TranspositionTable {
    private:
        std::vector<TTEntry> table;
        size_t size_mask;
        uint8_t current_age = 0;
        
    public:
        TranspositionTable(size_t size_mb = 64);
        
        void clear();
        void new_search() { current_age++; }
        
        bool probe(uint64_t key, TTEntry& entry) const;
        void store(uint64_t key, const S_MOVE& best_move, int score, int eval, 
                  int depth, TTFlag flag);
        
        size_t get_hashfull() const;
        void resize(size_t size_mb);
    };

    // Main search class
    class Engine {
    private:
        Position root_position;
        SearchLimits limits;
        SearchStats stats;
        MoveOrderer move_orderer;
        TranspositionTable tt;
        
        // Search control
        std::atomic<bool> stop_search{false};
        std::chrono::steady_clock::time_point search_start;
        
        // UCI communication
        std::function<void(const SearchInfo&)> info_callback;
        
        // Search variables
        int root_depth = 0;
        PVLine root_pv;
        
        // Search functions
        int alpha_beta(Position& pos, int alpha, int beta, int depth, int ply, PVLine& pv);
        int quiescence_search(Position& pos, int alpha, int beta, int ply);
        
        // Search helpers
        bool should_stop() const;
        bool is_mate_score(int score) const;
        int mate_distance(int score) const;
        void send_search_info(int depth, int score, const PVLine& pv) const;
        
        // Time management
        std::chrono::milliseconds calculate_search_time() const;
        bool time_up() const;
        
    public:
        Engine(size_t tt_size_mb = 64);
        
        // Main search interface
        S_MOVE search(const Position& pos, const SearchLimits& search_limits);
        void stop() { stop_search = true; }
        
        // Engine configuration
        void set_position(const Position& pos) { root_position = pos; }
        void clear_hash() { tt.clear(); move_orderer.clear(); }
        void resize_hash(size_t size_mb) { tt.resize(size_mb); }
        void set_hash_size(size_t size_mb) { resize_hash(size_mb); }
        
        // Search information callback
        void set_info_callback(std::function<void(const SearchInfo&)> callback) {
            info_callback = callback;
        }
        
        // Search information
        const SearchStats& get_stats() const { return stats; }
        const PVLine& get_pv() const { return root_pv; }
        size_t get_hashfull() const { return tt.get_hashfull(); }
        
        // UCI communication helpers
        void send_bestmove(const S_MOVE& move) const;
        void send_info(const std::string& info) const;
    };

    // Utility functions
    std::string move_to_uci(const S_MOVE& move);
    S_MOVE uci_to_move(const std::string& uci_move, const Position& pos);
    
    // Score to UCI format
    std::string score_to_uci(int score);
    
    // Time utilities
    std::chrono::milliseconds get_time_since(std::chrono::steady_clock::time_point start);

} // namespace Search
