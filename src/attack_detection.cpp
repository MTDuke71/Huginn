/**
 * @file attack_detection.cpp
 * @brief Square-attack detection for the pure-bitboard Huginn engine.
 */

#include "attack_detection.hpp"
#include "position.hpp"
#include "attack_tables.hpp"
#include "bitboard.hpp"
#include "msvc_optimizations.hpp"

#include <cassert>

namespace Huginn {

bool SqAttacked(int sq, const Position& pos, Color attacking_color) {
    return SqAttackedBB(sq, pos, attacking_color);
}

bool SqAttackedBB(int sq, const Position& pos, Color attacking_color) {
    assert(sq >= 0 && sq < 64);
    __assume(sq >= 0 && sq < 64);

    int color_idx = static_cast<int>(attacking_color);
    uint64_t enemy_pieces = pos.color_bitboards[color_idx];
    if (enemy_pieces == 0) return false;

    // 1. Pawn attacks (look in the OPPOSITE color's pawn-attack table from the target square)
    uint64_t enemy_pawns = pos.piece_bitboards[color_idx][static_cast<int>(PieceType::Pawn)];
    if (enemy_pawns != 0) {
        Color opposite_color = (attacking_color == Color::White) ? Color::Black : Color::White;
        uint64_t pawn_attackers = pawn_attacks[static_cast<int>(opposite_color)][sq];
        if ((pawn_attackers & enemy_pawns) != 0) return true;
    }

    // 2. Knight attacks
    uint64_t enemy_knights = pos.piece_bitboards[color_idx][static_cast<int>(PieceType::Knight)];
    if (enemy_knights != 0 && (knight_attacks[sq] & enemy_knights) != 0) return true;

    // 3. King attacks
    uint64_t enemy_kings = pos.piece_bitboards[color_idx][static_cast<int>(PieceType::King)];
    if (enemy_kings != 0 && (king_attacks[sq] & enemy_kings) != 0) return true;

    // 4. Rook + queen (rank/file sliders)
    uint64_t rook_attackers = pos.piece_bitboards[color_idx][static_cast<int>(PieceType::Rook)] |
                              pos.piece_bitboards[color_idx][static_cast<int>(PieceType::Queen)];
    if (rook_attackers != 0 && (rook_attacks(sq, pos.occupied_bitboard) & rook_attackers) != 0) return true;

    // 5. Bishop + queen (diagonal sliders)
    uint64_t bishop_attackers = pos.piece_bitboards[color_idx][static_cast<int>(PieceType::Bishop)] |
                                pos.piece_bitboards[color_idx][static_cast<int>(PieceType::Queen)];
    if (bishop_attackers != 0 && (bishop_attacks(sq, pos.occupied_bitboard) & bishop_attackers) != 0) return true;

    return false;
}

} // namespace Huginn
