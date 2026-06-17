/**
 * @file movegen_bb.hpp
 * @brief Bitboard move generation — Huginn's production move generator.
 *
 * Generates moves directly from per-piece attack bitboards (knight/king/pawn
 * lookup tables and magic-bitboard sliders) instead of iterating piece lists.
 * This is the path the search uses; the mailbox generator in movegen.* is
 * retained only for the @ref S_MOVELIST facade and cross-validation tests.
 *
 * @par Legality contract
 * The piece generators produce **pseudo-legal** moves: they exclude only
 * captures of friendly pieces, NOT moves that leave the side-to-move's king in
 * check. Full legality is enforced downstream by the make/unmake legality
 * filter (see Position::MakeMove). **Castling is the sole exception** — it is
 * emitted only when fully legal (clear path, and the king neither starts in,
 * passes through, nor lands on an attacked square).
 *
 * @par Special moves
 * Promotions (all four pieces, on both push and capture), double pawn pushes
 * (flagged via @ref make_pawn_start), en passant, and castling are all emitted.
 * Squares are indexed 0–63 (a1=0, h8=63); generators work natively in sq64.
 *
 * @author MTDuke71
 * @version 1.0
 */

#pragma once

#include "position.hpp"
#include "move.hpp"
#include "bitboard.hpp"
#include "attack_tables.hpp"

/**
 * @namespace BitboardMoveGen
 * @brief Bitboard-based move generation — the production move generator.
 *
 * Generates moves directly from per-piece attack bitboards (knight/king/pawn
 * lookup tables and magic-bitboard sliders) rather than iterating piece lists.
 * This is the path used by the search; the mailbox generator in movegen.* is
 * retained only for cross-validation and tests.
 */
namespace BitboardMoveGen {

/**
 * @brief Generate every pseudo-legal move for the side to move.
 * @param pos Position to generate from (side = `pos.side_to_move`).
 * @param[out] list Destination list; reset to empty, then populated.
 *
 * Clears @p list, then appends pawn, knight, bishop, rook, queen, king, and
 * castling moves. Output is pseudo-legal **except** castling, which is fully
 * legal (see the file-level legality contract). Each move carries its ordering
 * score (MVV-LVA on captures) via the S_MOVELIST `add_*` helpers. This is the
 * entry point the search calls.
 */
void generate_all_moves_bitboard(const Position& pos, S_MOVELIST& list);

/**
 * @brief Append pseudo-legal knight moves for @p us.
 * @param pos Source position.
 * @param[in,out] list List to append to (NOT cleared).
 * @param us Side whose knights to generate.
 *
 * `knight_attacks[sq]` lookup per knight, masked against own pieces; each
 * target square is split into a quiet or capture move.
 */
void generate_knight_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Append pseudo-legal pawn moves for @p us.
 * @param pos Source position.
 * @param[in,out] list List to append to (NOT cleared).
 * @param us Side whose pawns to generate.
 *
 * Single/double pushes via bitboard shifts and captures via
 * `pawn_attacks[us][sq]`. Emits all four under-promotions on push and capture,
 * and en-passant captures keyed off `pos.ep_square`.
 */
void generate_pawn_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Append pseudo-legal king (non-castling) moves for @p us.
 * @param pos Source position.
 * @param[in,out] list List to append to (NOT cleared).
 * @param us Side whose king to generate.
 *
 * `king_attacks[sq]` lookup masked against own pieces. Castling is emitted
 * separately by generate_all_moves_bitboard, not here.
 */
void generate_king_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Append pseudo-legal bishop moves for @p us.
 * @param pos Source position.
 * @param[in,out] list List to append to (NOT cleared).
 * @param us Side whose bishops to generate.
 *
 * `bishop_attacks(sq, occupied)` (magic bitboards) masked against own pieces.
 */
void generate_bishop_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Append pseudo-legal rook moves for @p us.
 * @param pos Source position.
 * @param[in,out] list List to append to (NOT cleared).
 * @param us Side whose rooks to generate.
 *
 * `rook_attacks(sq, occupied)` (magic bitboards) masked against own pieces.
 */
void generate_rook_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

/**
 * @brief Append pseudo-legal queen moves for @p us.
 * @param pos Source position.
 * @param[in,out] list List to append to (NOT cleared).
 * @param us Side whose queens to generate.
 *
 * `queen_attacks(sq, occupied)` = bishop ∪ rook attacks, masked against own
 * pieces.
 */
void generate_queen_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us);

} // namespace BitboardMoveGen