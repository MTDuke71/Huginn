/**
 * @file uci_utils.hpp
 * @brief UCI helper(s) shared by the UCI layer — currently move-string parsing.
 */
#pragma once
#include "movegen.hpp"
#include "position.hpp"
#include <string>

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
/// @return The per-move budget in ms, always >= 1.
long long compute_time_budget_ms(long long time_ms, long long inc_ms, long long movestogo);
