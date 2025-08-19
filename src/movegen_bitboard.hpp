#pragma once
#include "position.hpp"
#include "move.hpp"
#include "bitboard.hpp"
#include "attacks.hpp"
#include "sq_attacked.hpp"

inline void generate_all_moves_bitboard(const Position& pos, MoveList& out) {
    out.clear();
    const Color side = pos.side_to_move;
    const Bitboard friendly_pieces = pos.piece_bb[static_cast<int>(side)];
    const Bitboard enemy_pieces = pos.piece_bb[static_cast<int>(!side)];
    const Bitboard all_pieces = friendly_pieces | enemy_pieces;

    if (side == Color::White) {
        // Pawn moves
        Bitboard pawns = pos.get_piece_bb(PieceType::Pawn, Color::White);
        
        // Single pawn push
        Bitboard single_push = (pawns << 8) & ~all_pieces;
        
        // Double pawn push
        Bitboard double_push = ((single_push & RANK_3) << 8) & ~all_pieces;

        // Pawn captures
        Bitboard captures_west = (pawns << 7) & enemy_pieces & ~FILE_H;
        Bitboard captures_east = (pawns << 9) & enemy_pieces & ~FILE_A;

        // Add pawn moves to move list
        while (single_push) {
            int to = pop_lsb(single_push);
            int from = to - 8;
            out.add(make_move(from, to));
        }

        while (double_push) {
            int to = pop_lsb(double_push);
            int from = to - 16;
            out.add(make_move(from, to));
        }

        while (captures_west) {
            int to = pop_lsb(captures_west);
            int from = to - 7;
            out.add(make_move(from, to));
        }

        while (captures_east) {
            int to = pop_lsb(captures_east);
            int from = to - 9;
            out.add(make_move(from, to));
        }
    } else {
                // Black pawn moves
        Bitboard pawns = pos.get_piece_bb(PieceType::Pawn, Color::Black);

        // Single pawn push
        Bitboard single_push = (pawns >> 8) & ~all_pieces;

        // Double pawn push
        Bitboard double_push = ((single_push & RANK_6) >> 8) & ~all_pieces;

        // Pawn captures
        Bitboard captures_west = (pawns >> 9) & enemy_pieces & ~FILE_H;
        Bitboard captures_east = (pawns >> 7) & enemy_pieces & ~FILE_A;

        // Add pawn moves to move list
        while (single_push) {
            int to = pop_lsb(single_push);
            int from = to + 8;
            out.add(make_move(from, to));
        }

        while (double_push) {
            int to = pop_lsb(double_push);
            int from = to + 16;
            out.add(make_move(from, to));
        }

        while (captures_west) {
            int to = pop_lsb(captures_west);
            int from = to + 9;
            out.add(make_move(from, to));
        }

        while (captures_east) {
            int to = pop_lsb(captures_east);
            int from = to + 7;
            out.add(make_move(from, to));
        }
    }

    // Knight moves
    Bitboard knights = pos.get_piece_bb(PieceType::Knight, side);
    while (knights) {
        int from = pop_lsb(knights);
        Bitboard knight_moves = 0ULL;
        if (from > 18) knight_moves |= (1ULL << (from - 17));
        if (from > 10) knight_moves |= (1ULL << (from - 15));
        if (from < 54) knight_moves |= (1ULL << (from + 10));
        if (from < 46) knight_moves |= (1ULL << (from + 17));
        if (from > 20) knight_moves |= (1ULL << (from - 10));
        if (from > 12) knight_moves |= (1ULL << (from - 6));
        if (from < 57) knight_moves |= (1ULL << (from + 6));
        if (from < 49) knight_moves |= (1ULL << (from + 15));

        knight_moves &= ~friendly_pieces;

        while (knight_moves) {
            int to = pop_lsb(knight_moves);
            out.add(make_move(from, to));
        }
    }

    // Rook moves
    Bitboard rooks = pos.get_piece_bb(PieceType::Rook, side);
    while (rooks) {
        int from = pop_lsb(rooks);
        Bitboard rook_moves = Attacks::get_rook_attacks(from, all_pieces);
        rook_moves &= ~friendly_pieces;

        while (rook_moves) {
            int to = pop_lsb(rook_moves);
            out.add(make_move(from, to));
        }
    }

    // Bishop moves
    Bitboard bishops = pos.get_piece_bb(PieceType::Bishop, side);
    while (bishops) {
        int from = pop_lsb(bishops);
        Bitboard bishop_moves = Attacks::get_bishop_attacks(from, all_pieces);
        bishop_moves &= ~friendly_pieces;

        while (bishop_moves) {
            int to = pop_lsb(bishop_moves);
            out.add(make_move(from, to));
        }
    }

    // Queen moves
    Bitboard queens = pos.get_piece_bb(PieceType::Queen, side);
    while (queens) {
        int from = pop_lsb(queens);
        Bitboard queen_moves = Attacks::get_rook_attacks(from, all_pieces) | Attacks::get_bishop_attacks(from, all_pieces);
        queen_moves &= ~friendly_pieces;

        while (queen_moves) {
            int to = pop_lsb(queen_moves);
            out.add(make_move(from, to));
        }
    }

    // King moves
    Bitboard king = pos.get_piece_bb(PieceType::King, side);
    if (king) {
        int from = pop_lsb(king);
        Bitboard king_moves = 0ULL;
        if (from > 9) king_moves |= (1ULL << (from - 9));
        if (from > 8) king_moves |= (1ULL << (from - 8));
        if (from > 7) king_moves |= (1ULL << (from - 7));
        if (from > 1) king_moves |= (1ULL << (from - 1));
        if (from < 63) king_moves |= (1ULL << (from + 1));
        if (from < 56) king_moves |= (1ULL << (from + 7));
        if (from < 55) king_moves |= (1ULL << (from + 8));
        if (from < 54) king_moves |= (1ULL << (from + 9));

        king_moves &= ~friendly_pieces;

        while (king_moves) {
            int to = pop_lsb(king_moves);
            out.add(make_move(from, to));
        }
    }
}