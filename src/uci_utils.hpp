#pragma once
#include "movegen_enhanced.hpp"
#include "position.hpp"
#include "search.hpp"
#include <string>

// Utility function to parse UCI move string to S_MOVE
S_MOVE parse_uci_move(const std::string& uci_move, const Position& position);
