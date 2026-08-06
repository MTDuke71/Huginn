/**
 * @file uci_utils.hpp
 * @brief UCI helper(s) shared by the UCI layer — currently move-string parsing.
 */
#pragma once
#include "movegen.hpp"
#include "position.hpp"
#include "search.hpp"
#include <string>
#include <vector>

/// @brief Parse a UCI move string (e.g. "e2e4", "e7e8q") against @p position
///        into a fully-flagged S_MOVE by matching it to a generated legal move.
/// @param uci_move Long-algebraic move text.
/// @param position The position the move applies to (for flags / legality match).
/// @return The matching S_MOVE, or a null move (move == 0) if no legal move matches.
S_MOVE parse_uci_move(const std::string& uci_move, const Position& position);

/// @brief Structural / legal-position gate for the UCI `position` boundary
///        (BACKLOG #54). Rejects positions a syntactically valid FEN can still
///        describe but the engine cannot search safely: missing/duplicate
///        kings, pawns on back ranks, impossible piece counts (the BACKLOG #55
///        second boundary), the idle side in check, and castling/EP flags that
///        do not match the board. Deliberately a separate call so focused unit
///        tests can keep constructing partial positions via set_from_fen.
/// @param pos Position to inspect (as produced by set_from_fen).
/// @param reason Optional out-parameter receiving a short rejection reason.
/// @return true if the position is safe to hand to search.
bool validate_uci_position(const Position& pos, std::string* reason = nullptr);

/// @brief #56: strict numeric parser for UCI spin options and `go` parameters.
///        The whole token must be numeric (no suffix junk, no empty string),
///        with an overflow guard (saturates at ±1e9), then clamped to
///        [lo, hi]. Never throws — the old naked std::stoi terminated the
///        process on junk and prefix-parsed "12junk" as 12.
/// @return true and sets @p out on success; false on malformed input.
bool parse_spin_clamped(const std::string& s, long long lo, long long hi, long long& out);

/// @brief #56 (part 3): pure, 64-bit, boundary-safe per-move time allocation.
///        Strategy is unchanged from the gauntleted #47 tuning on normal
///        clocks — classical: time/movestogo + inc/2; sudden death:
///        time/20 + inc/2; hard caps at 60% of the clock and at
///        time − reserve where reserve = clamp(time/10, 50, 1000) — but the
///        floors are now sane: the 50 ms quality floor can never exceed the
///        safely usable remainder (the old final `max(50, alloc)` budgeted
///        50 ms with 1–10 ms on the clock), and a clock at/below the reserve
///        yields a 1 ms emergency budget instead of overdrawing.
/// @param time_ms   Side-to-move remaining clock in ms; negative = not sent
///                  (falls back to inc/4, floored at 50 ms, or 5000 ms if no
///                  increment either).
/// @param inc_ms    Side-to-move increment in ms (negative treated as 0).
/// @param movestogo Moves to the next time control; 0 = sudden death.
/// @return The per-move budget in ms, always >= 1. Under #65 this is the SOFT
///         bound (see compute_hard_time_budget_ms); its value is unchanged.
long long compute_time_budget_ms(long long time_ms, long long inc_ms, long long movestogo);

/// @brief BACKLOG #65: the HARD per-move deadline — the point at which an
///        in-flight search is aborted mid-iteration.
///
/// Huginn historically had ONE deadline: `compute_time_budget_ms` was both the
/// "don't start another iteration" point and the "abort now" point, so an
/// iteration that would have taken 10% longer than the budget was killed and
/// its work discarded. At 10+0.1 that pinned every move to <= 550 ms — across
/// 2455 measured moves Huginn never once exceeded 0.56 s, while Fable 0.8
/// (soft 408 / hard 1224) peaked at 1.23 s and used 95.5% of its clock to
/// Huginn's 83.2%.
///
/// Split: the soft bound still gates *starting* a new iteration; this hard
/// bound lets an iteration already begun run to completion, up to 3x soft.
/// The anti-forfeit guarantees are untouched — the same `safe_max` (clock
/// minus reserve) and `cap60` (60% of the clock) still bound the result, so
/// the hard deadline can never exceed what #47 already deemed safe to spend.
///
/// @return The hard deadline in ms; always >= the soft bound and >= 1.
long long compute_hard_time_budget_ms(long long time_ms, long long inc_ms, long long movestogo);

/// @brief BACKLOG #60 parser-purity refactor: pure parse of the UCI `go`
///        command tokens into search limits, with no side effects (does not
///        start a search). `handle_go` calls this and hands the result to
///        `search_best_move`; tests call it directly to assert exact parsed
///        results instead of wrapping a real search in `EXPECT_NO_THROW`.
/// @param tokens Tokenized `go` command (tokens[0] == "go"; parsing starts
///               at tokens[1]).
/// @param side_to_move Selects wtime/winc vs btime/binc for clock-based
///                      time allocation.
/// @param[out] infinite_requested Set true iff the literal `go infinite`
///             token was present — distinct from the returned limits'
///             `infinite` flag, which is also set for a depth-only search.
/// @return The computed MinimalLimits (mirrors the precedence `handle_go`
///         used: depth > movetime > clock-based > bare-`go` default).
Huginn::MinimalLimits parse_go_command(const std::vector<std::string>& tokens,
                                        Color side_to_move,
                                        bool& infinite_requested);
