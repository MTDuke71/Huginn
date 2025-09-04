#pragma once

#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "pvtable.hpp"
#include <chrono>

namespace Huginn {

// VICE Constants
const int INFINITE = 30000;
const int MATE = 29000;
// MAX_DEPTH is defined in pvtable.hpp

// Search info structure - equivalent to S_SEARCHINFO from VICE tutorial (0:19)
// Contains runtime search state and statistics
struct SearchInfo {
    std::chrono::steady_clock::time_point start_time;  // When search started
    std::chrono::steady_clock::time_point stop_time;   // When to stop search
    int depth;          // Current search depth
    int max_depth;      // Maximum depth to search (set from limits or manually in tests)
    int ply;            // Current search ply (VICE Part 57)
    int movestogo;      // Moves until next time control
    bool infinite;      // Search until told to stop
    bool quit;          // Flag to quit search
    bool stopped;       // Flag indicating search was stopped
    bool depth_only;    // UCI depth command - bypass time management
    uint64_t nodes;     // Nodes searched so far
    S_MOVE best_move;   // Best move found (VICE Part 58)
    
    // VICE Part 60: Move ordering statistics (0:13)
    uint64_t fh;        // Fail high count (beta cutoffs)
    uint64_t fhf;       // Fail high first (beta cutoff on first move)
    
    SearchInfo() : depth(0), max_depth(25), ply(0), movestogo(30), infinite(false), 
                   quit(false), stopped(false), depth_only(false), nodes(0), best_move(), fh(0), fhf(0) {}
};

// Search limits structure - external interface for setting search parameters
// Used by UCI and main interface to configure search behavior
struct MinimalLimits {
    int max_depth = 25;  // Maximum search depth (copied to SearchInfo.max_depth)
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
    
    // Search History array (3:55) - stores scores for moves that improved alpha
    // [piece][to_square] - 13 piece types, 120 squares (mailbox)
    int search_history[13][120];
    
    // Search Killers array (4:37) - stores non-capture moves causing beta cutoff  
    // [depth][killer_slot] - 64 levels, 2 killer moves per depth
    S_MOVE search_killers[64][2];
    
    // MVV-LVA (Most Valuable Victim, Least Valuable Attacker) table
    // [victim][attacker] - prioritizes captures where weak pieces take strong pieces
    // Higher scores = better captures (e.g., pawn takes queen = high score)
    int mvv_lva_scores[7][7];  // 7 piece types (None=0, Pawn=1, Knight=2, Bishop=3, Rook=4, Queen=5, King=6)
    
    // Constructor
    MinimalEngine() : pv_table(2) {
        clear_search_tables();
        init_mvv_lva();
    }
    
    // Clear search tables
    void clear_search_tables();
    
    // Simple material evaluation
    int evaluate(const Position& pos);
    
    // Helper functions for evaluation (Part 56)
    static int mirror_square_64(int sq64);
    
    // Basic alpha-beta search (VICE Part 58)
    int alpha_beta(Position& pos, int depth, int alpha, int beta, SearchInfo& info, bool doNull = true);
    
    // Time checking
    bool time_up() const;
    void check_up(SearchInfo& info);
    
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
    int get_pv_line(Position& pos, int depth, S_MOVE pv_array[64]);
    
    // Search history and killer move functions
    void update_search_history(const Position& pos, const S_MOVE& move, int depth);
    void update_killer_moves(const S_MOVE& move, int depth);
    
    // MVV-LVA (Most Valuable Victim, Least Valuable Attacker) functions
    void init_mvv_lva();                                              // Initialize MVV-LVA scoring table
    int get_mvv_lva_score(PieceType victim, PieceType attacker) const; // Get capture score
    void order_moves(std::vector<S_MOVE>& moves, const Position& pos) const; // Order moves using MVV-LVA
    void order_moves(S_MOVELIST& move_list, const Position& pos) const;      // Order moves in S_MOVELIST
    
    // VICE Part 62: Move Picking - Pick best move from remaining moves
    int pick_next_move(S_MOVELIST& move_list, int move_num, const Position& pos) const;
    
    // VICE Part 55 - Search Function Definitions
    static void checkup(SearchInfo& info);                            // Check time limits and GUI interrupts (1:34)
    static void clearForSearch(MinimalEngine& engine, SearchInfo& info);  // Clear search tables and PV (2:25) - VICE Part 57
    int AlphaBeta(Position& pos, int alpha, int beta, int depth, SearchInfo& info, bool doNull, bool isRoot = false);  // Core search (2:58)
    int quiescence(Position& pos, int alpha, int beta, SearchInfo& info);  // Quiescence search (4:40)
    int evalPosition(const Position& pos);                   // Position evaluation (0:34)
    
    // VICE-style search function that demonstrates clearForSearch usage (Part 57)
    S_MOVE searchPosition(Position& pos, SearchInfo& info);
};

} // namespace Huginn
