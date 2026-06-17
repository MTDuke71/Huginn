/**
 * @file movegen_bb.cpp
 * @brief Bitboard move-generation implementation (see movegen_bb.hpp).
 *
 * Per-piece generators read the side's piece bitboard, derive targets from the
 * attack tables / magic sliders, mask off friendly occupancy, and split each
 * target into a quiet or capture move. Pawns additionally handle pushes,
 * promotions, and en passant; castling is a file-local helper. Output is
 * pseudo-legal except castling — full legality is the caller's job (see the
 * legality contract in movegen_bb.hpp). Moves use 64-square indices; the
 * generators work natively in sq64.
 *
 * @author MTDuke71
 * @version 1.0
 */

#include "movegen_bb.hpp"
#include "square.hpp"
#include "attack_detection.hpp"
#include "chess_types.hpp"

namespace BitboardMoveGen {

// File-local: defined at end of namespace (only consumer is
// generate_all_moves_bitboard below).
static void generate_castling_moves_optimized(const Position& pos, S_MOVELIST& list, Color us);

void generate_all_moves_bitboard(const Position& pos, S_MOVELIST& list) {
    list.count = 0;
    Color us = pos.side_to_move;

    // Use pure bitboard generation for all pieces
    generate_pawn_moves_bitboard(pos, list, us);
    generate_knight_moves_bitboard(pos, list, us);
    generate_bishop_moves_bitboard(pos, list, us);
    generate_rook_moves_bitboard(pos, list, us);
    generate_queen_moves_bitboard(pos, list, us);
    generate_king_moves_bitboard(pos, list, us);

    generate_castling_moves_optimized(pos, list, us);
}

void generate_knight_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us) {
    // Get knight bitboard - this is the key difference from piece lists!
    uint64_t knights = pos.piece_bitboards[int(us)][int(PieceType::Knight)];
    uint64_t own_pieces = pos.color_bitboards[int(us)];

    // Iterate through knights using bitboard operations
    while (knights != 0) {
        int from_sq64 = get_lsb(knights);
        knights &= knights - 1;  // Remove processed knight

        // Get all knight attacks for this square - O(1) lookup!
        uint64_t attacks = knight_attacks[from_sq64];

        // Remove our own pieces from targets
        attacks &= ~own_pieces;

        // Generate moves for each target square
        while (attacks != 0) {
            int to_sq64 = get_lsb(attacks);
            attacks &= attacks - 1;  // Remove processed target

            // Check if it's a capture or quiet move
            Piece target = pos.at_sq64(to_sq64);
            if (target == Piece::None) {
                list.add_quiet_move(make_move(from_sq64, to_sq64));
            } else {
                list.add_capture_move(make_capture(from_sq64, to_sq64, type_of(target)), pos);
            }
        }
    }
}

void generate_pawn_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us) {
    uint64_t pawns = pos.piece_bitboards[int(us)][int(PieceType::Pawn)];
    uint64_t occupied = pos.occupied_bitboard;
    uint64_t enemies = pos.color_bitboards[int(!us)];

    if (us == Color::White) {
        // White pawn pushes (shift north)
        uint64_t single_pushes = (pawns << 8) & ~occupied;  // Shift north, remove blocked
        uint64_t double_pushes = ((single_pushes & 0xFF0000ULL) << 8) & ~occupied;  // From rank 3

        // Generate single pushes
        uint64_t pushes = single_pushes;
        while (pushes != 0) {
            int to_sq64 = get_lsb(pushes);
            pushes &= pushes - 1;

            int from_sq64 = to_sq64 - 8;  // One rank south

            // Check for promotion
            if (to_sq64 >= 56) {  // 8th rank
                list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Queen));
                list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Rook));
                list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Bishop));
                list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Knight));
            } else {
                list.add_quiet_move(make_move(from_sq64, to_sq64));
            }
        }

        // Generate double pushes
        pushes = double_pushes;
        while (pushes != 0) {
            int to_sq64 = get_lsb(pushes);
            pushes &= pushes - 1;

            int from_sq64 = to_sq64 - 16;  // Two ranks south

            list.add_quiet_move(make_pawn_start(from_sq64, to_sq64));
        }

        // White pawn captures using attack bitboards
        uint64_t pawn_copy = pos.piece_bitboards[int(us)][int(PieceType::Pawn)];
        while (pawn_copy != 0) {
            int from_sq64 = get_lsb(pawn_copy);
            pawn_copy &= pawn_copy - 1;

            // Get pawn attacks and filter to enemy pieces
            uint64_t attacks = pawn_attacks[int(Color::White)][from_sq64] & enemies;

            while (attacks != 0) {
                int to_sq64 = get_lsb(attacks);
                attacks &= attacks - 1;

                Piece captured = pos.at_sq64(to_sq64);

                if (to_sq64 >= 56) {  // Promotion capture
                    list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Queen, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Rook, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Bishop, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Knight, type_of(captured)));
                } else {
                    list.add_capture_move(make_capture(from_sq64, to_sq64, type_of(captured)), pos);
                }
            }
        }
    } else {
        // Black pawn moves (similar but shift south)
        uint64_t single_pushes = (pawns >> 8) & ~occupied;  // Shift south
        uint64_t double_pushes = ((single_pushes & 0xFF0000000000ULL) >> 8) & ~occupied;  // From rank 6

        // Generate single pushes
        uint64_t pushes = single_pushes;
        while (pushes != 0) {
            int to_sq64 = get_lsb(pushes);
            pushes &= pushes - 1;

            int from_sq64 = to_sq64 + 8;  // One rank north

            if (to_sq64 <= 7) {  // 1st rank promotion
                list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Queen));
                list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Rook));
                list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Bishop));
                list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Knight));
            } else {
                list.add_quiet_move(make_move(from_sq64, to_sq64));
            }
        }

        // Generate double pushes
        pushes = double_pushes;
        while (pushes != 0) {
            int to_sq64 = get_lsb(pushes);
            pushes &= pushes - 1;

            int from_sq64 = to_sq64 + 16;  // Two ranks north

            list.add_quiet_move(make_pawn_start(from_sq64, to_sq64));
        }

        // Black pawn captures
        uint64_t pawn_copy = pos.piece_bitboards[int(us)][int(PieceType::Pawn)];
        while (pawn_copy != 0) {
            int from_sq64 = get_lsb(pawn_copy);
            pawn_copy &= pawn_copy - 1;

            uint64_t attacks = pawn_attacks[int(Color::Black)][from_sq64] & enemies;

            while (attacks != 0) {
                int to_sq64 = get_lsb(attacks);
                attacks &= attacks - 1;

                Piece captured = pos.at_sq64(to_sq64);

                if (to_sq64 <= 7) {  // Promotion capture
                    list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Queen, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Rook, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Bishop, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq64, to_sq64, PieceType::Knight, type_of(captured)));
                } else {
                    list.add_capture_move(make_capture(from_sq64, to_sq64, type_of(captured)), pos);
                }
            }
        }
    }

    // Generate en passant captures
    if (pos.ep_square != -1) {
        int ep_sq64 = pos.ep_square;  // ep_square is now sq64

        if (ep_sq64 >= 0) {  // Valid 64-square position
            uint64_t ep_attackers = 0;

            if (us == Color::White) {
                // White pawns can capture en passant if they're on rank 5 (sq64: 32-39)
                // and can attack the en passant square
                ep_attackers = pawn_attacks[int(Color::Black)][ep_sq64] & pawns;
            } else {
                // Black pawns can capture en passant if they're on rank 4 (sq64: 24-31)
                // and can attack the en passant square
                ep_attackers = pawn_attacks[int(Color::White)][ep_sq64] & pawns;
            }

            while (ep_attackers != 0) {
                int from_sq64 = get_lsb(ep_attackers);
                ep_attackers &= ep_attackers - 1;

                list.add_en_passant_move(make_en_passant(from_sq64, ep_sq64));
            }
        }
    }
}

void generate_king_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us) {
    uint64_t king = pos.piece_bitboards[int(us)][int(PieceType::King)];
    if (king == 0) return;

    int king_sq64 = get_lsb(king);

    // Get king attacks - O(1) lookup
    uint64_t attacks = king_attacks[king_sq64];
    uint64_t own_pieces = pos.color_bitboards[int(us)];

    // Remove own pieces
    attacks &= ~own_pieces;

    while (attacks != 0) {
        int to_sq64 = get_lsb(attacks);
        attacks &= attacks - 1;

        Piece target = pos.at_sq64(to_sq64);

        if (target == Piece::None) {
            list.add_quiet_move(make_move(king_sq64, to_sq64));
        } else {
            list.add_capture_move(make_capture(king_sq64, to_sq64, type_of(target)), pos);
        }
    }
}

void generate_bishop_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us) {
    uint64_t bishops = pos.piece_bitboards[int(us)][int(PieceType::Bishop)];
    uint64_t own_pieces = pos.color_bitboards[int(us)];
    uint64_t occupied = pos.occupied_bitboard;

    while (bishops != 0) {
        int from_sq64 = get_lsb(bishops);
        bishops &= bishops - 1;

        // Get bishop attacks with occupancy
        uint64_t attacks = bishop_attacks(from_sq64, occupied) & ~own_pieces;

        while (attacks != 0) {
            int to_sq64 = get_lsb(attacks);
            attacks &= attacks - 1;

            Piece target = pos.at_sq64(to_sq64);

            if (target == Piece::None) {
                list.add_quiet_move(make_move(from_sq64, to_sq64));
            } else {
                list.add_capture_move(make_capture(from_sq64, to_sq64, type_of(target)), pos);
            }
        }
    }
}

void generate_rook_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us) {
    uint64_t rooks = pos.piece_bitboards[int(us)][int(PieceType::Rook)];
    uint64_t own_pieces = pos.color_bitboards[int(us)];
    uint64_t occupied = pos.occupied_bitboard;

    while (rooks != 0) {
        int from_sq64 = get_lsb(rooks);
        rooks &= rooks - 1;

        // Get rook attacks with occupancy
        uint64_t attacks = rook_attacks(from_sq64, occupied) & ~own_pieces;

        while (attacks != 0) {
            int to_sq64 = get_lsb(attacks);
            attacks &= attacks - 1;

            Piece target = pos.at_sq64(to_sq64);

            if (target == Piece::None) {
                list.add_quiet_move(make_move(from_sq64, to_sq64));
            } else {
                list.add_capture_move(make_capture(from_sq64, to_sq64, type_of(target)), pos);
            }
        }
    }
}

void generate_queen_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us) {
    uint64_t queens = pos.piece_bitboards[int(us)][int(PieceType::Queen)];
    uint64_t own_pieces = pos.color_bitboards[int(us)];
    uint64_t occupied = pos.occupied_bitboard;

    while (queens != 0) {
        int from_sq64 = get_lsb(queens);
        queens &= queens - 1;

        // Get queen attacks (bishop + rook) with occupancy
        uint64_t attacks = queen_attacks(from_sq64, occupied) & ~own_pieces;

        while (attacks != 0) {
            int to_sq64 = get_lsb(attacks);
            attacks &= attacks - 1;

            Piece target = pos.at_sq64(to_sq64);

            if (target == Piece::None) {
                list.add_quiet_move(make_move(from_sq64, to_sq64));
            } else {
                list.add_capture_move(make_capture(from_sq64, to_sq64, type_of(target)), pos);
            }
        }
    }
}

/**
 * @brief Append fully-legal castling moves for @p us (file-local).
 * @param pos Source position.
 * @param[in,out] list List to append to.
 * @param us Side to generate castling for.
 *
 * The one generator that emits **legal**, not pseudo-legal, moves: it requires
 * the castling right, the king on its start square, an empty path, the correct
 * rook present, and (via Huginn::SqAttackedBB) that the king's start,
 * transit, and destination squares are all unattacked. Moved here from the
 * removed king_lookup_tables module; the sole caller is
 * generate_all_moves_bitboard.
 */
static void generate_castling_moves_optimized(const Position& pos, S_MOVELIST& list, Color us) {
    // Calculate castle squares using the same logic as CastlingSquares
    constexpr int WHITE_KING_START = sq64(File::E, Rank::R1);
    constexpr int WHITE_KINGSIDE_KING_TO = sq64(File::G, Rank::R1);
    constexpr int WHITE_QUEENSIDE_KING_TO = sq64(File::C, Rank::R1);
    constexpr int BLACK_KING_START = sq64(File::E, Rank::R8);
    constexpr int BLACK_KINGSIDE_KING_TO = sq64(File::G, Rank::R8);
    constexpr int BLACK_QUEENSIDE_KING_TO = sq64(File::C, Rank::R8);

    if (us == Color::White) {
        const int e1 = WHITE_KING_START;
        // Castling requires king on its starting square
        if (pos.at_sq64(e1) != Piece::WhiteKing) return;

        // White kingside castling (e1-g1)
        if (pos.castling_rights & CASTLE_WK) {
            const int f1 = sq64(File::F, Rank::R1);
            const int g1 = sq64(File::G, Rank::R1);
            const int h1 = sq64(File::H, Rank::R1);

            if (pos.at_sq64(h1) == Piece::WhiteRook &&
                pos.at_sq64(f1) == Piece::None && pos.at_sq64(g1) == Piece::None) {
                if (!Huginn::SqAttackedBB(e1, pos, Color::Black) &&
                    !Huginn::SqAttackedBB(f1, pos, Color::Black) &&
                    !Huginn::SqAttackedBB(g1, pos, Color::Black)) {
                    list.add_castle_move(make_castle(e1, WHITE_KINGSIDE_KING_TO));
                }
            }
        }

        // White queenside castling (e1-c1)
        if (pos.castling_rights & CASTLE_WQ) {
            const int d1 = sq64(File::D, Rank::R1);
            const int c1 = sq64(File::C, Rank::R1);
            const int b1 = sq64(File::B, Rank::R1);
            const int a1 = sq64(File::A, Rank::R1);

            if (pos.at_sq64(a1) == Piece::WhiteRook &&
                pos.at_sq64(d1) == Piece::None && pos.at_sq64(c1) == Piece::None && pos.at_sq64(b1) == Piece::None) {
                if (!Huginn::SqAttackedBB(e1, pos, Color::Black) &&
                    !Huginn::SqAttackedBB(d1, pos, Color::Black) &&
                    !Huginn::SqAttackedBB(c1, pos, Color::Black)) {
                    list.add_castle_move(make_castle(e1, WHITE_QUEENSIDE_KING_TO));
                }
            }
        }
    } else {
        const int e8 = BLACK_KING_START;
        if (pos.at_sq64(e8) != Piece::BlackKing) return;

        // Black kingside castling (e8-g8)
        if (pos.castling_rights & CASTLE_BK) {
            const int f8 = sq64(File::F, Rank::R8);
            const int g8 = sq64(File::G, Rank::R8);
            const int h8 = sq64(File::H, Rank::R8);

            if (pos.at_sq64(h8) == Piece::BlackRook &&
                pos.at_sq64(f8) == Piece::None && pos.at_sq64(g8) == Piece::None) {
                if (!Huginn::SqAttackedBB(e8, pos, Color::White) &&
                    !Huginn::SqAttackedBB(f8, pos, Color::White) &&
                    !Huginn::SqAttackedBB(g8, pos, Color::White)) {
                    list.add_castle_move(make_castle(e8, BLACK_KINGSIDE_KING_TO));
                }
            }
        }

        // Black queenside castling (e8-c8)
        if (pos.castling_rights & CASTLE_BQ) {
            const int d8 = sq64(File::D, Rank::R8);
            const int c8 = sq64(File::C, Rank::R8);
            const int b8 = sq64(File::B, Rank::R8);
            const int a8 = sq64(File::A, Rank::R8);

            if (pos.at_sq64(a8) == Piece::BlackRook &&
                pos.at_sq64(d8) == Piece::None && pos.at_sq64(c8) == Piece::None && pos.at_sq64(b8) == Piece::None) {
                if (!Huginn::SqAttackedBB(e8, pos, Color::White) &&
                    !Huginn::SqAttackedBB(d8, pos, Color::White) &&
                    !Huginn::SqAttackedBB(c8, pos, Color::White)) {
                    list.add_castle_move(make_castle(e8, BLACK_QUEENSIDE_KING_TO));
                }
            }
        }
    }
}

} // namespace BitboardMoveGen
