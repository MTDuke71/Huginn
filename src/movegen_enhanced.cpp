#include "movegen_enhanced.hpp"
#include "position.hpp"
#include "move.hpp"
#include "movegen.hpp"  // For SqAttacked function
#include "board120.hpp"
#include "chess_types.hpp"

// Enhanced move generation function with improved organization
void generate_all_moves(const Position& pos, S_MOVELIST& list) {
    list.clear();
    
    Color us = pos.side_to_move;
    
    // Generate moves for each piece type using piece lists (fastest approach)
    generate_pawn_moves(pos, list, us);
    generate_knight_moves(pos, list, us);
    generate_bishop_moves(pos, list, us);
    generate_rook_moves(pos, list, us);
    generate_queen_moves(pos, list, us);
    generate_king_moves(pos, list, us);
}

// Specialized move generation functions for each piece type
void generate_pawn_moves(const Position& pos, S_MOVELIST& list, Color us) {
    int piece_count = pos.pCount[int(us)][int(PieceType::Pawn)];
    
    for (int i = 0; i < piece_count; ++i) {
        int from = pos.pList[int(us)][int(PieceType::Pawn)][i];
        if (from == -1) continue;  // Safety check
        
        // Determine pawn direction and starting rank
        int direction = (us == Color::White) ? NORTH : SOUTH;
        Rank start_rank = (us == Color::White) ? Rank::R2 : Rank::R7;
        Rank promo_rank = (us == Color::White) ? Rank::R8 : Rank::R1;
        
        // Single push
        int to = from + direction;
        if (is_playable(to) && pos.at(to) == Piece::None) {
            if (rank_of(to) == promo_rank) {
                // Promotions
                list.add_promotion_move(make_promotion(from, to, PieceType::Queen));
                list.add_promotion_move(make_promotion(from, to, PieceType::Rook));
                list.add_promotion_move(make_promotion(from, to, PieceType::Bishop));
                list.add_promotion_move(make_promotion(from, to, PieceType::Knight));
            } else {
                list.add_quiet_move(make_move(from, to));
                
                // Double push from starting position
                if (rank_of(from) == start_rank) {
                    int double_to = to + direction;
                    if (is_playable(double_to) && pos.at(double_to) == Piece::None) {
                        list.add_quiet_move(make_pawn_start(from, double_to));
                    }
                }
            }
        }
        
        // Captures (diagonals)
        for (int capture_dir : {direction + WEST, direction + EAST}) {
            int capture_to = from + capture_dir;
            if (is_playable(capture_to)) {
                Piece target = pos.at(capture_to);
                
                if (target != Piece::None && color_of(target) == !us) {
                    // Regular capture
                    if (rank_of(capture_to) == promo_rank) {
                        // Capture promotion
                        list.add_promotion_move(make_promotion(from, capture_to, PieceType::Queen, type_of(target)));
                        list.add_promotion_move(make_promotion(from, capture_to, PieceType::Rook, type_of(target)));
                        list.add_promotion_move(make_promotion(from, capture_to, PieceType::Bishop, type_of(target)));
                        list.add_promotion_move(make_promotion(from, capture_to, PieceType::Knight, type_of(target)));
                    } else {
                        list.add_capture_move(make_capture(from, capture_to, type_of(target)), pos);
                    }
                } else if (capture_to == pos.ep_square) {
                    // En passant capture
                    list.add_en_passant_move(make_en_passant(from, capture_to));
                }
            }
        }
    }
}

void generate_knight_moves(const Position& pos, S_MOVELIST& list, Color us) {
    int piece_count = pos.pCount[int(us)][int(PieceType::Knight)];
    
    for (int i = 0; i < piece_count; ++i) {
        int from = pos.pList[int(us)][int(PieceType::Knight)][i];
        if (from == -1) continue;
        
        for (int delta : KNIGHT_DELTAS) {
            int to = from + delta;
            if (is_playable(to)) {
                Piece target = pos.at(to);
                
                if (target == Piece::None) {
                    list.add_quiet_move(make_move(from, to));
                } else if (color_of(target) == !us) {
                    list.add_capture_move(make_capture(from, to, type_of(target)), pos);
                }
            }
        }
    }
}

void generate_sliding_moves(const Position& pos, S_MOVELIST& list, Color us, PieceType piece_type, const int* directions, int num_directions) {
    int piece_count = pos.pCount[int(us)][int(piece_type)];
    
    for (int i = 0; i < piece_count; ++i) {
        int from = pos.pList[int(us)][int(piece_type)][i];
        if (from == -1) continue;
        
        for (int d = 0; d < num_directions; ++d) {
            int direction = directions[d];
            
            for (int to = from + direction; is_playable(to); to += direction) {
                Piece target = pos.at(to);
                
                if (target == Piece::None) {
                    list.add_quiet_move(make_move(from, to));
                } else {
                    if (color_of(target) == !us) {
                        list.add_capture_move(make_capture(from, to, type_of(target)), pos);
                    }
                    break;  // Piece blocks further movement
                }
            }
        }
    }
}

void generate_bishop_moves(const Position& pos, S_MOVELIST& list, Color us) {
    static const int directions[] = {NE, NW, SE, SW};
    generate_sliding_moves(pos, list, us, PieceType::Bishop, directions, 4);
}

void generate_rook_moves(const Position& pos, S_MOVELIST& list, Color us) {
    static const int directions[] = {NORTH, SOUTH, EAST, WEST};
    generate_sliding_moves(pos, list, us, PieceType::Rook, directions, 4);
}

void generate_queen_moves(const Position& pos, S_MOVELIST& list, Color us) {
    static const int directions[] = {NORTH, SOUTH, EAST, WEST, NE, NW, SE, SW};
    generate_sliding_moves(pos, list, us, PieceType::Queen, directions, 8);
}

void generate_king_moves(const Position& pos, S_MOVELIST& list, Color us) {
    int king_sq = pos.king_sq[int(us)];
    if (king_sq == -1) return;  // Safety check
    
    // Regular king moves
    for (int delta : KING_DELTAS) {
        int to = king_sq + delta;
        if (is_playable(to)) {
            Piece target = pos.at(to);
            
            if (target == Piece::None) {
                list.add_quiet_move(make_move(king_sq, to));
            } else if (color_of(target) == !us) {
                list.add_capture_move(make_capture(king_sq, to, type_of(target)), pos);
            }
        }
    }
    
    // Castling moves
    if (us == Color::White) {
        // White kingside castling
        if ((pos.castling_rights & CASTLE_WK) && 
            pos.at(sq(File::F, Rank::R1)) == Piece::None &&
            pos.at(sq(File::G, Rank::R1)) == Piece::None &&
            !SqAttacked(sq(File::E, Rank::R1), pos, Color::Black) &&
            !SqAttacked(sq(File::F, Rank::R1), pos, Color::Black) &&
            !SqAttacked(sq(File::G, Rank::R1), pos, Color::Black)) {
            list.add_castle_move(make_castle(sq(File::E, Rank::R1), sq(File::G, Rank::R1)));
        }
        
        // White queenside castling
        if ((pos.castling_rights & CASTLE_WQ) &&
            pos.at(sq(File::D, Rank::R1)) == Piece::None &&
            pos.at(sq(File::C, Rank::R1)) == Piece::None &&
            pos.at(sq(File::B, Rank::R1)) == Piece::None &&
            !SqAttacked(sq(File::E, Rank::R1), pos, Color::Black) &&
            !SqAttacked(sq(File::D, Rank::R1), pos, Color::Black) &&
            !SqAttacked(sq(File::C, Rank::R1), pos, Color::Black)) {
            list.add_castle_move(make_castle(sq(File::E, Rank::R1), sq(File::C, Rank::R1)));
        }
    } else {
        // Black kingside castling
        if ((pos.castling_rights & CASTLE_BK) &&
            pos.at(sq(File::F, Rank::R8)) == Piece::None &&
            pos.at(sq(File::G, Rank::R8)) == Piece::None &&
            !SqAttacked(sq(File::E, Rank::R8), pos, Color::White) &&
            !SqAttacked(sq(File::F, Rank::R8), pos, Color::White) &&
            !SqAttacked(sq(File::G, Rank::R8), pos, Color::White)) {
            list.add_castle_move(make_castle(sq(File::E, Rank::R8), sq(File::G, Rank::R8)));
        }
        
        // Black queenside castling
        if ((pos.castling_rights & CASTLE_BQ) &&
            pos.at(sq(File::D, Rank::R8)) == Piece::None &&
            pos.at(sq(File::C, Rank::R8)) == Piece::None &&
            pos.at(sq(File::B, Rank::R8)) == Piece::None &&
            !SqAttacked(sq(File::E, Rank::R8), pos, Color::White) &&
            !SqAttacked(sq(File::D, Rank::R8), pos, Color::White) &&
            !SqAttacked(sq(File::C, Rank::R8), pos, Color::White)) {
            list.add_castle_move(make_castle(sq(File::E, Rank::R8), sq(File::C, Rank::R8)));
        }
    }
}

// Enhanced legal move generation with better performance
void generate_legal_moves_enhanced(const Position& pos, S_MOVELIST& list) {
    S_MOVELIST pseudo_moves;
    generate_all_moves(pos, pseudo_moves);
    
    list.clear();
    
    // Filter out illegal moves (those that leave king in check)
    for (int i = 0; i < pseudo_moves.count; ++i) {
        Position temp_pos = pos;  // Copy position
        S_UNDO undo;
        temp_pos.make_move_with_undo(pseudo_moves[i], undo);
        
        // Check if our king is still in check after the move
        Color us = pos.side_to_move;
        if (!SqAttacked(temp_pos.king_sq[int(us)], temp_pos, !us)) {
            list.moves[list.count] = pseudo_moves[i];
            list.count++;
        }
        temp_pos.undo_move();
    }
}
