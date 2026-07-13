/**
 * @file search.hpp
 * @brief Search engine interface — the Engine class and its runtime state.
 *
 * Declares the alpha-beta search and everything it needs: the Engine (which owns
 * the transposition table, PV table, opening book, tablebase handle, and the
 * move-ordering tables), the per-search ::SearchInfo state/statistics block, and
 * the ::MinimalLimits configuration passed in from UCI.
 *
 * The search is negamax alpha-beta with PVS, iterative deepening, a full pruning
 * stack (null-move, RFP, futility, razoring, LMR), check extensions, IID, and
 * quiescence with SEE pruning. Move ordering blends TT/PV/IID hints, MVV-LVA
 * captures, killers, the butterfly history table, and the counter-move heuristic.
 * The hand-crafted evaluation (`evaluate`) lives in search.cpp alongside the
 * search. See [SEARCH_AND_EVAL.md] for the audited technique inventory.
 *
 * @see position.hpp for the board representation the search operates on.
 * @see transposition_table.hpp, pvtable.hpp for the search-owned tables.
 */
#pragma once

#include "position.hpp"
#include "move.hpp"
#include "movegen.hpp"
#include "pvtable.hpp"
#include "transposition_table.hpp"
#include "polyglot_book.hpp"
#include "syzygy_tablebase.hpp"
#include <atomic>
#include <chrono>
#include <functional>
#include <vector>

namespace Huginn {

// BACKLOG #3: 1-ply continuation history (counter-move history). Generalizes
// the scalar counter-move table (ENABLE_PLY_TRACKED_COUNTERMOVE) into a full
// depth^2-updated history conditioned on the parent move. Gated for clean SPRT
// A/B: when 0, the table, helpers, and call sites all vanish. See search.cpp
// ordering/update sites and clear_search_tables() for the zero-init.
// Default OFF: the 1-ply additive implementation was falsified by gauntlet
// (w16 neutral, w64 -9 Elo Intel / worse AMD — see BACKLOG #3). Kept gated and
// in-tree for a future bounded-gravity redesign; flag-off is byte-identical to
// baseline-t9. Set to 1 only to re-test a revised design.
#ifndef ENABLE_CONTINUATION_HISTORY
#define ENABLE_CONTINUATION_HISTORY 0
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
constexpr int CONTHIST_ORDER_WEIGHT = 64;
constexpr int CONTHIST_ORDER_CAP = 8000;

// Engine-internal diagnostic counters gate. When 1, search emits a
// second per-depth `info string` with non-standard counters (null-move
// cuts, LMR attempt/failure ratio, TT hit/miss/write counters) AND
// the per-node increments that feed them are compiled in. When 0, the
// emission and ALL the increments compile out, saving a handful of
// uint64_t ops per node in the hot path. The standard UCI `info` line
// (depth/seldepth/multipv/score/nodes/nps/hashfull/tbhits/time/pv) is
// always emitted regardless. Flip to 1 only when tuning pruning/LMR
// or debugging TT behaviour.
//
// Same gate must be visible to transposition_table.hpp (which carries
// the hits/misses/writes counters); both files #ifndef-guard so the
// CLI `-DENABLE_INFO_DIAGNOSTICS=1` works uniformly.
#ifndef ENABLE_INFO_DIAGNOSTICS
#define ENABLE_INFO_DIAGNOSTICS 0
#endif

// VICE Constants
const int INFINITE = 30000;
const int MATE = 29000;
// MAX_DEPTH is defined in pvtable.hpp

/**
 * @brief Per-search runtime state, limits, statistics, and the collected PV.
 *
 * One instance lives for the duration of a `searchPosition` call. It carries the
 * time-control window, depth/ply counters, the node/tbhit tallies, the move-
 * ordering diagnostic counters (fail-high, null/futility/LMR/razoring cuts), the
 * per-ply search stack used by the counter-move heuristic, and the triangular PV
 * table that records the exact principal variation. Equivalent to VICE's
 * `S_SEARCHINFO`.
 */
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
    int seldepth;       // Max selective depth seen (incl. qsearch); standard UCI info
    uint64_t tbhits;    // Successful Syzygy tablebase probes; standard UCI info
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

    // Singular extensions (ENABLE_SINGULAR_EXT): TT moves extended after the
    // exclusion search failed low. Always present (tests read it on the ON
    // arm); never incremented on the baseline arm.
    uint64_t singular_exts;

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

    // #56: mid-search input pump installed by the UCI layer. When checkup()
    // sees pending stdin input it calls this instead of read_input(); the UCI
    // pump answers `isready` with `readyok` WITHOUT stopping the search,
    // applies `stop`/`quit` to this SearchInfo, and queues everything else
    // (`position`, `go`, `setoption`, ...) for the command loop to replay in
    // order after `bestmove`. Empty (bare-Engine use: tests, bench) falls back
    // to read_input()'s conservative any-input-stops behaviour.
    std::function<void(SearchInfo&)> on_input;

    SearchInfo() : depth(0), max_depth(25), ply(0), movestogo(30), infinite(false),
                   quit(false), stopped(false), depth_only(false), nodes(0), seldepth(0), tbhits(0),
                   best_move(), fh(0), fhf(0), null_cut(0),
                   futility_cuts(0), lmr_attempts(0), lmr_failures(0), razoring_cuts(0),
                   singular_exts(0) {
        // Initialize search stack
        for (int i = 0; i < 64; ++i) {
            search_stack[i] = S_MOVE();
            pv_length[i] = 0;
        }
    }
};

/**
 * @brief Caller-facing search configuration (depth / time / infinite).
 *
 * The external interface UCI and tests use to bound a search; copied into the
 * ::SearchInfo at the start of `searchPosition`.
 */
struct MinimalLimits {
    int max_depth = 25;     ///< Maximum search depth (copied to SearchInfo::max_depth).
    int max_time_ms = 5000; ///< Soft time budget for the move, in milliseconds.
    bool infinite = false;  ///< If true, search until explicitly stopped (ignore the time budget).
};

/**
 * @brief The search engine: owns the search tables and runs alpha-beta.
 *
 * A single Engine drives one search position at a time. It owns the
 * transposition table, PV table, opening book, optional Syzygy tablebase handle,
 * and the per-search learning tables (history, killers, counter-moves, MVV-LVA).
 * `searchPosition` is the top-level entry point; `AlphaBeta`/`quiescence` are the
 * recursive core; `evaluate` is the hand-crafted leaf evaluation. Members are
 * public for test access.
 */
class Engine {
public:  // Make members public for easier access
    /**
     * @brief Constructs the engine and initializes its tables.
     * @param tb Optional Syzygy tablebase handle (nullptr = tablebases disabled).
     */
    Engine(SyzygyTablebase* tb = nullptr) : tablebase(tb), pv_table(2), tt_table(64) {
        // Initialize MVV-LVA table
        init_mvv_lva();
        
        // Clear search tables
        clear_search_tables();
    }
    
    // #56: the ONE cross-thread cancellation channel. stop()/signal_stop()
    // (any thread) set it; checkup() polls it and translates it into
    // info.stopped ON the searching thread, so SearchInfo itself is never
    // written cross-thread (the old path published a raw pointer to the
    // stack SearchInfo and wrote its non-atomic fields from another thread).
    std::atomic<bool> should_stop{false};
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
    
    /// Ages (history) and clears (killers/counters) the per-search ordering tables.
    void clear_search_tables();

    /**
     * @brief Hand-crafted static evaluation of a position.
     * @param pos Position to score.
     * @return Score in centipawns from the side-to-move's perspective (+ = better
     *         for the side to move). Tapered material+PSTs plus pawn structure,
     *         mobility, threats, king safety, etc. (definition in search.cpp).
     */
    int evaluate(const Position& pos);

    /**
     * @brief Tests for an insufficient-material draw (e.g. K vs K, K+minor vs K).
     * @return true if neither side can possibly mate. (VICE Part 82)
     */
    static bool MaterialDraw(const Position& pos);

    /**
     * @brief Returns a color-mirrored copy of a position (for eval-symmetry tests).
     * @param pos Position to mirror.
     * @return The vertically-flipped, color-swapped position; `evaluate` of it must
     *         equal `evaluate` of the original for a symmetric eval.
     */
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
    // BACKLOG #48: tt_move (0 = none) is supplied by the caller's TT probe —
    // no re-probe inside; AlphaBeta already probed at node entry.
    int pick_next_move(S_MOVELIST& move_list, int move_num, const Position& pos, const SearchInfo& info, int depth = -1, const S_MOVE& iid_move = S_MOVE{}, uint32_t tt_move = 0) const;
    
    // VICE Part 85: Opening book functions
    bool load_opening_book(const std::string& book_path);
    S_MOVE get_book_move(const Position& pos) const;
    
    // ---- VICE Part 55: core search ----

    /// Polls the clock and GUI input; sets `info.stopped` when the budget is hit. (1:34)
    void checkup(SearchInfo& info);
    /// Resets per-search tables, counters, and the PV before a fresh search. (VICE Part 57)
    static void clearForSearch(Engine& engine, SearchInfo& info);

    /**
     * @brief The recursive negamax alpha-beta search (the core of the engine).
     * @param pos Position to search (mutated and restored in place).
     * @param alpha Lower bound of the search window.
     * @param beta Upper bound of the search window.
     * @param depth Remaining depth in plies.
     * @param info Shared per-search state/statistics.
     * @param doNull Whether null-move pruning is permitted at this node.
     * @param isRoot True at the root (enables root-specific handling).
     * @param excluded_move Singular-extension exclusion search (ENABLE_SINGULAR_EXT):
     *        when non-zero, this node searches the same position *minus* this move
     *        (no TT cutoff, no TT store — the entry describes the full move set).
     *        0 (the default) everywhere else; inert on the baseline arm.
     * @return The negamax score from the side-to-move's perspective.
     */
    int AlphaBeta(Position& pos, int alpha, int beta, int depth, SearchInfo& info, bool doNull, bool isRoot = false, uint32_t excluded_move = 0);

    /**
     * @brief Quiescence search — extends the leaf with captures/checks until quiet.
     * @param pos Position to search.
     * @param alpha Lower window bound.
     * @param beta Upper window bound.
     * @param info Shared per-search state.
     * @param q_depth Current quiescence depth (capped at MAX_QUIESCENCE_DEPTH).
     * @return The stabilized (stand-pat or capture-refined) score.
     */
    int quiescence(Position& pos, int alpha, int beta, SearchInfo& info, int q_depth = 0);

    /// Thin wrapper around evaluate() used as the search leaf evaluation. (0:34)
    int evalPosition(const Position& pos);

    /**
     * @brief Internal Iterative Deepening: derive an ordering move for a PV node
     *        that has no TT/hash move, via a shallow search.
     * @return The best move from the reduced-depth search, used only for ordering.
     */
    S_MOVE internal_iterative_deepening(Position& pos, int alpha, int beta, int depth, SearchInfo& info);

    /**
     * @brief Top-level search entry point: iterative deepening from the root.
     * @param pos Position to search from.
     * @param info Limits in / statistics and best move out.
     * @return The best move found (also left in `info.best_move`). (VICE Part 57)
     */
    S_MOVE searchPosition(Position& pos, SearchInfo& info);

    /**
     * @brief Probes Syzygy WDL tablebases at a leaf.
     * @param[out] wdl_score Win/draw/loss score on Huginn's scale, if a hit.
     * @return true if the position was found in the tablebases.
     */
    bool probe_tablebase_wdl(const Position& pos, int& wdl_score) const;

    /**
     * @brief Probes Syzygy tablebases at the root for the best move.
     * @return The tablebase-optimal move, or an empty move if no hit.
     */
    S_MOVE probe_tablebase_root(const Position& pos) const;
};

} // namespace Huginn
