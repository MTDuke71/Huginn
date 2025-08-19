#include "movegen_simple.hpp"
#include "board120.hpp"
#include "sq_attacked.hpp"

void generate_legal_moves_simple(const Position& pos, MoveList& out) {
    out.clear();
    MoveList pseudo_legal; // First generate all pseudo-legal moves

    for (int from = 0; from < 120; ++from) {
        if (!is_playable(from)) continue;

        Piece piece = pos.at(from);
        if (is_none(piece) || color_of(piece) != pos.side_to_move) continue;

        PieceType pt = type_of(piece);
        Color c = color_of(piece);

        if (pt == PieceType::Pawn) {
            // Pawn moves
            int dir = (c == Color::White) ? 10 : -10;
            int start_rank = (c == Color::White) ? 1 : 6;
            int promotion_rank = (c == Color::White) ? 7 : 0;

            // Single push
            int to = from + dir;
            if (is_playable(to) && is_none(pos.at(to))) {
                if (static_cast<int>(rank_of(to)) == promotion_rank) {
                    pseudo_legal.add(make_promotion(from, to, PieceType::Queen));
                    pseudo_legal.add(make_promotion(from, to, PieceType::Rook));
                    pseudo_legal.add(make_promotion(from, to, PieceType::Bishop));
                    pseudo_legal.add(make_promotion(from, to, PieceType::Knight));
                } else {
                    pseudo_legal.add(make_move(from, to));
                }

                // Double push
                if (static_cast<int>(rank_of(from)) == start_rank) {
                    to = from + 2 * dir;
                    if (is_playable(to) && is_none(pos.at(to))) {
                        pseudo_legal.add(make_pawn_start(from, to));
                    }
                }
            }

            // Captures
            for (int capture_dir : { dir - 1, dir + 1 }) {
                to = from + capture_dir;
                if (is_playable(to) && !is_none(pos.at(to)) && color_of(pos.at(to)) != c) {
                    if (static_cast<int>(rank_of(to)) == promotion_rank) {
                        pseudo_legal.add(make_promotion(from, to, PieceType::Queen, type_of(pos.at(to))));
                        pseudo_legal.add(make_promotion(from, to, PieceType::Rook, type_of(pos.at(to))));
                        pseudo_legal.add(make_promotion(from, to, PieceType::Bishop, type_of(pos.at(to))));
                        pseudo_legal.add(make_promotion(from, to, PieceType::Knight, type_of(pos.at(to))));
                    } else {
                        pseudo_legal.add(make_capture(from, to, type_of(pos.at(to))));
                    }
                }
            }

            // En passant
            if (pos.ep_square != -1) {
                if (pos.ep_square == from + dir - 1 || pos.ep_square == from + dir + 1) {
                    pseudo_legal.add(make_en_passant(from, pos.ep_square));
                }
            }
        } else {
            // Other pieces
            for (int delta : PIECE_DELTAS[static_cast<int>(pt)]) {
                int to = from + delta;
                while (is_playable(to)) {
                    Piece captured_piece = pos.at(to);
                    if (is_none(captured_piece)) {
                        pseudo_legal.add(make_move(from, to));
                    } else {
                        if (color_of(captured_piece) != c) {
                            pseudo_legal.add(make_capture(from, to, type_of(captured_piece)));
                        }
                        break;
                    }
                    if (pt == PieceType::Knight || pt == PieceType::King) {
                        break;
                    }
                    to += delta;
                }
            }
        }
    }
    
    // Generate castling moves
    Color us = pos.side_to_move;
    Color them = !us;
    
    if (us == Color::White) {
        // White castling
        int king_sq = sq(File::E, Rank::R1);
        
        // Kingside castling (short castling)
        if ((pos.castling_rights & CASTLE_WK) && 
            is_none(pos.at(sq(File::F, Rank::R1))) && 
            is_none(pos.at(sq(File::G, Rank::R1))) &&
            !SqAttacked(king_sq, pos, them) &&
            !SqAttacked(sq(File::F, Rank::R1), pos, them) &&
            !SqAttacked(sq(File::G, Rank::R1), pos, them)) {
            pseudo_legal.add(make_castle(king_sq, sq(File::G, Rank::R1)));
        }
        
        // Queenside castling (long castling)
        if ((pos.castling_rights & CASTLE_WQ) && 
            is_none(pos.at(sq(File::D, Rank::R1))) && 
            is_none(pos.at(sq(File::C, Rank::R1))) &&
            is_none(pos.at(sq(File::B, Rank::R1))) &&
            !SqAttacked(king_sq, pos, them) &&
            !SqAttacked(sq(File::D, Rank::R1), pos, them) &&
            !SqAttacked(sq(File::C, Rank::R1), pos, them)) {
            pseudo_legal.add(make_castle(king_sq, sq(File::C, Rank::R1)));
        }
    } else {
        // Black castling
        int king_sq = sq(File::E, Rank::R8);
        
        // Kingside castling (short castling)
        if ((pos.castling_rights & CASTLE_BK) && 
            is_none(pos.at(sq(File::F, Rank::R8))) && 
            is_none(pos.at(sq(File::G, Rank::R8))) &&
            !SqAttacked(king_sq, pos, them) &&
            !SqAttacked(sq(File::F, Rank::R8), pos, them) &&
            !SqAttacked(sq(File::G, Rank::R8), pos, them)) {
            pseudo_legal.add(make_castle(king_sq, sq(File::G, Rank::R8)));
        }
        
        // Queenside castling (long castling)
        if ((pos.castling_rights & CASTLE_BQ) && 
            is_none(pos.at(sq(File::D, Rank::R8))) && 
            is_none(pos.at(sq(File::C, Rank::R8))) &&
            is_none(pos.at(sq(File::B, Rank::R8))) &&
            !SqAttacked(king_sq, pos, them) &&
            !SqAttacked(sq(File::D, Rank::R8), pos, them) &&
            !SqAttacked(sq(File::C, Rank::R8), pos, them)) {
            pseudo_legal.add(make_castle(king_sq, sq(File::C, Rank::R8)));
        }
    }

    // Now filter out illegal moves (moves that leave the king in check)
    for (const auto& move : pseudo_legal.v) {
        // Make a copy of the position to test the move
        Position test_pos = pos;
        State st{};
        make_move(test_pos, move, st);
        
        // Check if our king is in check after this move
        int our_king_sq = test_pos.king_sq[static_cast<int>(pos.side_to_move)];
        bool king_in_check = SqAttacked(our_king_sq, test_pos, !pos.side_to_move);
        
        // If the king is not in check, this is a legal move
        if (!king_in_check) {
            out.add(move);
        }
        
        // Unmake the move
        unmake_move(test_pos, move, st);
    }
}