#pragma once
#include "chess_types.hpp"

class Position;

namespace Huginn {

/// Bitboard-based attack detection. @p sq is 0..63 (a1=0, h8=63 in little-endian rank-file).
bool SqAttackedBB(int sq, const Position& pos, Color attacking_color);

} // namespace Huginn
