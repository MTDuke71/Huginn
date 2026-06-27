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

/**
 * @brief Piece values for SEE, indexed by PieceType — an **INDEPENDENT** table,
 *        not a view of eval's PIECE_VALUES_MG (see INVARIANTS.md: three value
 *        tables). SEE wants stable, exchange-fair, single-phase values and must
 *        NOT follow eval's tuning; it starts equal to the classic MG set but is
 *        free to diverge. King = 20000 so it's only ever the "least valuable
 *        attacker" when the destination has no enemy recapture.
 */
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
