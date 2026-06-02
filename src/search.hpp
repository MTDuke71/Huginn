#pragma once

#include "position.hpp"
#include "move.hpp"
#include "movegen.hpp"
#include "pvtable.hpp"
#include "transposition_table.hpp"
#include "polyglot_book.hpp"
#include "syzygy_tablebase.hpp"
#include <chrono>
#include <vector>

namespace Huginn {

// BACKLOG #3: 1-ply continuation history (counter-move history). Generalizes
// the scalar counter-move table (ENABLE_PLY_TRACKED_COUNTERMOVE) into a full
// depth^2-updated history conditioned on the parent move. Gated for clean SPRT
// A/B: when 0, the table, helpers, and call sites all vanish. See search.cpp
// ordering/update sites and clear_search_tables() for the zero-init.
#ifndef ENABLE_CONTINUATION_HISTORY
#define ENABLE_CONTINUATION_HISTORY 1
#endif

// BACKLOG #3 tuning knobs (the PRIMARY levers for the conthist gauntlet):
//   WEIGHT — multiplies the conthist score before blending into the quiet-move
//     ordering score. At weight 1 the term is inert: Huginn's butterfly history
//     accumulates unbounded over all parent contexts (~1K+ per entry, per the
//     counter-move comment) and swamps a single-context conthist entry. ~16
//     brings conthist to a co-equal secondary signal; tune by gauntlet.
//   CAP — clamps |weight*conthist| so a hot quiet move stays in the history
//     band and never leaps the promotion (25K-90K) / killer (800K-900K) /
//     capture (1M) tiers, preserving the existing ordering hierarchy.
constexpr int CONTHIST_ORDER_WEIGHT = 16;
constexpr int CONTHIST_ORDER_CAP = 8000;

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
    
    // VICE Part 83: Null move pruning statistics
    uint64_t null_cut;  // Null move cutoffs
    
    /**
     * @brief Futility Pruning Statistics
     * 
     * Futility pruning is a forward pruning technique that skips evaluation
     * of moves in positions where even the best possible move cannot 
     * significantly improve the position's score relative to alpha.
     * 
     * Applied at shallow depths (≤3) with safety margins:
     * - Base margin: 100 centipawns
     * - Additional margin: 50cp per remaining ply
     * - Safety: Never prunes when in check or at root
     * 
     * Provides 5-15% search speedup while maintaining tactical accuracy.
     */
    uint64_t futility_cuts;  // Futility pruning cutoffs
    
    // Late Move Reduction statistics
    uint64_t lmr_attempts;  // Number of LMR reductions attempted
    uint64_t lmr_failures;  // Number of LMR reductions that failed high (needed re-search)
    
    // Advanced search optimization statistics
    uint64_t razoring_cuts;  // Razoring depth reductions applied

    // Counter-move heuristic: track moves played to update counter-move table
    S_MOVE search_stack[64];  // Stack of moves made during search (max 64 plies)

    // Triangular PV table — the exact principal variation collected during the
    // search itself. pv_line[ply] holds the PV from that ply onward and
    // pv_length[ply] its length; a node copies its best child's line up and
    // prepends its own move. This replaces reconstructing the PV from the
    // side pv_table afterward, which truncated badly under heavy node counts
    // (the overwrite-always table evicted continuation entries before the line
    // could be read back, collapsing deep PVs to a single move). Indexed by
    // info.ply (0..63).
    S_MOVE pv_line[64][64];
    int pv_length[64];

    SearchInfo() : depth(0), max_depth(25), ply(0), movestogo(30), infinite(false),
                   quit(false), stopped(false), depth_only(false), nodes(0), best_move(), fh(0), fhf(0), null_cut(0),
                   futility_cuts(0), lmr_attempts(0), lmr_failures(0), razoring_cuts(0) {
        // Initialize search stack
        for (int i = 0; i < 64; ++i) {
            search_stack[i] = S_MOVE();
            pv_length[i] = 0;
        }
    }
};

// Search limits structure - external interface for setting search parameters
// Used by UCI and main interface to configure search behavior
struct MinimalLimits {
    int max_depth = 25;  // Maximum search depth (copied to SearchInfo.max_depth)
    int max_time_ms = 5000;
    bool infinite = false;
};

class Engine {
public:  // Make members public for easier access
    // Constructor
    Engine(SyzygyTablebase* tb = nullptr) : tablebase(tb), pv_table(2), tt_table(64) {
        // Initialize MVV-LVA table
        init_mvv_lva();
        
        // Clear search tables
        clear_search_tables();
    }
    
    bool should_stop = false;
    int nodes_searched = 0;
    std::chrono::steady_clock::time_point start_time;
    MinimalLimits current_limits;
    PVTable pv_table;  // Principal Variation table (VICE tutorial style)
    TranspositionTable tt_table;  // VICE Part 84: Transposition table for storing search results
    PolyglotBook opening_book;    // VICE Part 85: Polyglot opening book for opening moves
    SyzygyTablebase* tablebase;  // Syzygy tablebase for endgame perfect play
    
    // Search History array (3:55) - stores scores for moves that improved alpha
    // [piece][to_square] - 13 piece types, 64 squares
    // Zero-initialized: clear_search_tables() *ages* (/4) rather than zeroing,
    // so without this the first search reads per-process garbage, making quiet
    // move ordering (and thus results in many-equal-move positions) depend on
    // uninitialized memory — nondeterministic run-to-run (BACKLOG #30).
    int search_history[13][64] = {};
    
    // Search Killers array (4:37) - stores non-capture moves causing beta cutoff  
    // [depth][killer_slot] - 64 levels, 2 killer moves per depth
    S_MOVE search_killers[64][2];
    
    // Counter-Move Heuristic (5-15% search speedup)
    // [from_square][to_square] - maps opponent's last move to best counter-move
    // When move X causes beta-cutoff, store move Y as good counter to previous opponent move
    // 64x64 table (~32KB memory) for temporary per-search learning
    S_MOVE counter_moves[64][64];

#if ENABLE_CONTINUATION_HISTORY
    // BACKLOG #3: 1-ply continuation history (counter-move history).
    // Logically [prevPiece][prevTo][curPiece][curTo] -> score, flattened to a
    // heap vector and indexed via ch_index(). The "prev" piece/to is the
    // opponent's last move; at the current node that piece sits on prevTo, so
    // it is recoverable via pos.at_sq64(prevTo). Updated with the same +/-depth^2
    // bonus as butterfly history, blended additively into quiet-move ordering.
    //
    // HEAP-allocated (not a by-value array): at 13*64*13*64 int16 = ~1.38MB it
    // would overflow the 1MB Windows stack when a test constructs Engine by
    // value. int16_t (not int) halves the footprint — Huginn is footprint-
    // sensitive (the board64 cache #26 was reverted on that basis).
    // Zero-initialized at construction; clear_search_tables() ages rather than
    // zeroing, so the first search never reads garbage (nondeterminism, cf. #30).
    static constexpr int CH_PIECES = 13;
    static constexpr int CH_SQUARES = 64;
    static constexpr size_t CH_SIZE =
        size_t(CH_PIECES) * CH_SQUARES * CH_PIECES * CH_SQUARES;
    std::vector<int16_t> continuation_history = std::vector<int16_t>(CH_SIZE, 0);

    static constexpr size_t ch_index(int pp, int prev_to, int cp, int to) {
        return ((size_t(pp) * CH_SQUARES + prev_to) * CH_PIECES + cp) * CH_SQUARES + to;
    }
#endif

    // MVV-LVA (Most Valuable Victim, Least Valuable Attacker) table
    // [victim][attacker] - prioritizes captures where weak pieces take strong pieces
    // Higher scores = better captures (e.g., pawn takes queen = high score)
    int mvv_lva_scores[7][7];  // 7 piece types (None=0, Pawn=1, Knight=2, Bishop=3, Rook=4, Queen=5, King=6)
    
    // Clear search tables
    void clear_search_tables();
    
    // Simple material evaluation
    int evaluate(const Position& pos);
    
    // VICE Part 82: Material draw detection (2:03)
    static bool MaterialDraw(const Position& pos);
    
    // VICE Tutorial: Mirror Board function for evaluation testing
    // Creates a mirrored copy of the position for symmetry testing
    Position mirrorBoard(const Position& pos);


    // UCI score formatting helper
    // UCI score formatting
    std::string format_uci_score(int score, Color side_to_move) const;

    void stop() { should_stop = true; }
    void reset() { should_stop = false; nodes_searched = 0; }
    
    // Utility to convert move to UCI string
    static std::string move_to_uci(const S_MOVE& move);
    
    // Simple repetition detection (VICE tutorial style)
    static bool isRepetition(const Position& pos);
    
    // PV table helper functions
    void store_pv_move(uint64_t position_key, const S_MOVE& move);

    // Search history and killer move functions
    void update_search_history(const Position& pos, const S_MOVE& move, int depth);
    void penalize_search_history(const Position& pos, const S_MOVE& move, int depth);
    void age_search_history();
    void update_killer_moves(const S_MOVE& move, int depth);
    
    // Counter-move heuristic functions (5-15% search speedup)
    void update_counter_move(const S_MOVE& previous_move, const S_MOVE& counter_move);
    S_MOVE get_counter_move(const S_MOVE& previous_move) const;

#if ENABLE_CONTINUATION_HISTORY
    // BACKLOG #3: 1-ply continuation history. `prev` is the parent move
    // (info.search_stack[ply-1]); both helpers must be called with `pos` at the
    // current node (after TakeMove), where prev's piece sits on prev.get_to().
    void update_continuation_history(const Position& pos, const S_MOVE& prev,
                                     const S_MOVE& move, int bonus);
    int get_continuation_history(const Position& pos, const S_MOVE& prev,
                                 const S_MOVE& move) const;
#endif

    // MVV-LVA (Most Valuable Victim, Least Valuable Attacker) functions
    void init_mvv_lva();                                              // Initialize MVV-LVA scoring table
    int get_mvv_lva_score(PieceType victim, PieceType attacker) const; // Get capture score
    void order_moves(std::vector<S_MOVE>& moves, const Position& pos) const; // Order moves using MVV-LVA
    void order_moves(S_MOVELIST& move_list, const Position& pos) const;      // Order moves in S_MOVELIST
    
    // VICE Part 62: Move Picking - Pick best move from remaining moves  
    // VICE Part 64: Enhanced with PV move, killer moves, and history heuristic
    // Counter-move heuristic: Enhanced with counter-move support
    // IID Enhancement: Enhanced with Internal Iterative Deepening move support
    int pick_next_move(S_MOVELIST& move_list, int move_num, const Position& pos, const SearchInfo& info, int depth = -1, const S_MOVE& iid_move = S_MOVE{}) const;
    
    // VICE Part 85: Opening book functions
    bool load_opening_book(const std::string& book_path);
    S_MOVE get_book_move(const Position& pos) const;
    
    // VICE Part 55 - Search Function Definitions
    void checkup(SearchInfo& info);                            // Check time limits and GUI interrupts (1:34)
    static void clearForSearch(Engine& engine, SearchInfo& info);  // Clear search tables and PV (2:25) - VICE Part 57
    int AlphaBeta(Position& pos, int alpha, int beta, int depth, SearchInfo& info, bool doNull, bool isRoot = false);  // Core search (2:58)
    int quiescence(Position& pos, int alpha, int beta, SearchInfo& info, int q_depth = 0);  // Quiescence search with depth limit
    int evalPosition(const Position& pos);                   // Position evaluation (0:34)
    
    // Internal Iterative Deepening for PV nodes without hash move
    S_MOVE internal_iterative_deepening(Position& pos, int alpha, int beta, int depth, SearchInfo& info);
    
    // VICE-style search function that demonstrates clearForSearch usage (Part 57)
    S_MOVE searchPosition(Position& pos, SearchInfo& info);
    
    // Syzygy tablebase functions
    bool probe_tablebase_wdl(const Position& pos, int& wdl_score) const;
    S_MOVE probe_tablebase_root(const Position& pos) const;
};

} // namespace Huginn
