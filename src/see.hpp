#pragma once
/**
 * @file see.hpp
 * @brief Static Exchange Evaluation — net material gain/loss of a sequence
 *        of recaptures on the destination square of a candidate move.
 *
 * Used for two purposes in the search:
 *   1. Move ordering: split MVV-LVA captures into SEE >= 0 (good trades)
 *      and SEE < 0 (losing trades) buckets so losing captures sink below
 *      quiet moves in the move-pick order.
 *   2. Quiescence pruning: skip captures with SEE < 0 entirely; they
 *      can only worsen the standing eval, so searching them adds noise.
 *
 * Implementation: standard iterative swap-out with bitboard attacker
 * enumeration. Re-derives sliding (bishop/rook/queen) x-ray attackers
 * through removed pieces in the swap chain.
 */

#include <cstdint>
#include "move.hpp"
#include "position.hpp"

namespace Huginn {

// Piece values for SEE — an INDEPENDENT table, not a view of eval's
// PIECE_VALUES_MG. SEE is a static exchange estimate for capture
// pruning/ordering and wants stable, exchange-fair, single-phase values;
// it should NOT follow eval's Texel/SPSA tuning. The values happen to
// start equal to the MG set but are free to diverge and are maintained
// here separately on purpose. King is huge so the king is only ever the
// "least valuable attacker" when no enemy recapture exists (i.e. capturing
// with king is never a swap initiator when the destination is defended).
inline constexpr int SEE_PIECE_VALUE[7] = { 0, 100, 320, 330, 500, 900, 20000 };

/**
 * @brief Compute the SEE score of a capture or capture-promotion.
 * @param pos   Position from which `move` is being considered (not made).
 * @param move  The candidate capture.
 * @return Net material gain in centipawns from the side-to-move's POV.
 *         >= 0 means the trade is at least equal; < 0 means losing.
 *
 * Caller is expected to only invoke this for is_capture() moves; for
 * non-captures the result is 0 and meaningless.
 */
int see(const Position& pos, const S_MOVE& move);

} // namespace Huginn
