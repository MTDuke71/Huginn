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
