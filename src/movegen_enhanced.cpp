#include "movegen_enhanced.hpp"
#include "position.hpp"
#include "move.hpp"
#include "attack_detection.hpp"  // For SqAttacked function
#include "board120.hpp"
#include "chess_types.hpp"
#include "pawn_optimizations.hpp"
#include "king_optimizations.hpp"
#include "knight_optimizations.hpp"
#include "sliding_piece_optimizations.hpp"

// Enhanced move generation function with improved organization
void generate_all_moves(const Position& pos, S_MOVELIST& list) {
    list.count = 0;  // Direct clear - faster than function call
    
    Color us = pos.side_to_move;
    
    // Generate moves for each piece type using piece lists (fastest approach)
    // Use optimized pawn generation (addresses 20.3% of generation time)
    PawnOptimizations::generate_pawn_moves_optimized(pos, list, us);
    KnightOptimizations::generate_knight_moves_template(pos, list, us);
    // Use optimized sliding piece generation (addresses 45%+ of generation time)
    SlidingPieceOptimizations::generate_all_sliding_moves_optimized(pos, list, us);
    KingOptimizations::generate_king_moves_optimized(pos, list, us);
}

// ====================================================================
// Individual Piece Move Generation Functions
// ====================================================================
// NOTE: These functions are NOT used in production move generation.
// They are retained for:
// - Profiling system (movegen_profiler.cpp) - measures individual piece performance
// - Test validation - ensures optimized versions produce identical results
// - Development analysis - allows isolated testing of specific piece types
//
// Production code uses optimized combined functions:
// - PawnOptimizations::generate_pawn_moves_optimized()
// - KnightOptimizations::generate_knight_moves_template()
// - SlidingPieceOptimizations::generate_all_sliding_moves_optimized()
// - KingOptimizations::generate_king_moves_optimized()
// ====================================================================

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
        if (IS_PLAYABLE(to) && pos.at(to) == Piece::None) {
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
                    if (IS_PLAYABLE(double_to) && pos.at(double_to) == Piece::None) {
                        list.add_quiet_move(make_pawn_start(from, double_to));
                    }
                }
            }
        }
        
        // Captures (diagonals)
        for (int capture_dir : {direction + WEST, direction + EAST}) {
            int capture_to = from + capture_dir;
            if (IS_PLAYABLE(capture_to)) {
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
            if (IS_PLAYABLE(to)) {
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
            
            for (int to = from + direction; IS_PLAYABLE(to); to += direction) {
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

// ====================================================================
// Legacy Sliding Piece Functions (NOT used in production)
// ====================================================================
// These functions are retained for profiling and testing only.
// Production uses: SlidingPieceOptimizations::generate_all_sliding_moves_optimized()
// ====================================================================

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
        if (IS_PLAYABLE(to)) {
            Piece target = pos.at(to);
            
            if (target == Piece::None) {
                list.add_quiet_move(make_move(king_sq, to));
            } else if (color_of(target) == !us) {
                list.add_capture_move(make_capture(king_sq, to, type_of(target)), pos);
            }
        }
    }
    
    // Castling moves - only if king is on starting square
    if (us == Color::White) {
        // White castling - king must be on e1
        if (king_sq == sq(File::E, Rank::R1)) {
            // White kingside castling - rook must be on h1
            if ((pos.castling_rights & CASTLE_WK) && 
                pos.at(sq(File::H, Rank::R1)) == Piece::WhiteRook &&
                pos.at(sq(File::F, Rank::R1)) == Piece::None &&
                pos.at(sq(File::G, Rank::R1)) == Piece::None &&
                !SqAttacked(sq(File::E, Rank::R1), pos, Color::Black) &&
                !SqAttacked(sq(File::F, Rank::R1), pos, Color::Black) &&
                !SqAttacked(sq(File::G, Rank::R1), pos, Color::Black)) {
                list.add_castle_move(make_castle(sq(File::E, Rank::R1), sq(File::G, Rank::R1)));
            }
            
            // White queenside castling - rook must be on a1
            if ((pos.castling_rights & CASTLE_WQ) &&
                pos.at(sq(File::A, Rank::R1)) == Piece::WhiteRook &&
                pos.at(sq(File::D, Rank::R1)) == Piece::None &&
                pos.at(sq(File::C, Rank::R1)) == Piece::None &&
                pos.at(sq(File::B, Rank::R1)) == Piece::None &&
                !SqAttacked(sq(File::E, Rank::R1), pos, Color::Black) &&
                !SqAttacked(sq(File::D, Rank::R1), pos, Color::Black) &&
                !SqAttacked(sq(File::C, Rank::R1), pos, Color::Black)) {
                list.add_castle_move(make_castle(sq(File::E, Rank::R1), sq(File::C, Rank::R1)));
            }
        }
    } else {
        // Black castling - king must be on e8
        if (king_sq == sq(File::E, Rank::R8)) {
            // Black kingside castling - rook must be on h8
            if ((pos.castling_rights & CASTLE_BK) &&
                pos.at(sq(File::H, Rank::R8)) == Piece::BlackRook &&
                pos.at(sq(File::F, Rank::R8)) == Piece::None &&
                pos.at(sq(File::G, Rank::R8)) == Piece::None &&
                !SqAttacked(sq(File::E, Rank::R8), pos, Color::White) &&
                !SqAttacked(sq(File::F, Rank::R8), pos, Color::White) &&
                !SqAttacked(sq(File::G, Rank::R8), pos, Color::White)) {
                list.add_castle_move(make_castle(sq(File::E, Rank::R8), sq(File::G, Rank::R8)));
            }
            
            // Black queenside castling - rook must be on a8
            if ((pos.castling_rights & CASTLE_BQ) &&
                pos.at(sq(File::A, Rank::R8)) == Piece::BlackRook &&
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
}

// Enhanced legal move generation with better performance
void generate_legal_moves_enhanced(Position& pos, S_MOVELIST& list) {
    S_MOVELIST pseudo_moves;
    generate_all_moves(pos, pseudo_moves);
    
    list.count = 0;  // Direct clear - faster than function call
    
    // BUGFIX: Work on a copy to avoid modifying the input position
    Position temp_pos = pos;
    
    // Filter out illegal moves (those that leave king in check)
    for (int i = 0; i < pseudo_moves.size(); ++i) {
        if (temp_pos.MakeMove(pseudo_moves[i]) == 1) {
            // Check if our king is still in check after the move
            Color us = !temp_pos.side_to_move; // Side to move is now the opponent
            if (!SqAttacked(temp_pos.king_sq[int(us)], temp_pos, !us)) {
                list.add_quiet_move(pseudo_moves[i]);  // Use proper API
            }
            temp_pos.TakeMove();
        }
    }
}

// =============================================================================
// BACKWARD COMPATIBILITY IMPLEMENTATIONS
// =============================================================================
