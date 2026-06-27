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
