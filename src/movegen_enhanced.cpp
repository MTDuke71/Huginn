/**
 * @file movegen_enhanced.cpp
 * @brief Enhanced chess move generation with performance optimizations
 * 
 * Implements high-performance move generation for the Huginn chess engine using
 * specialized optimization modules for each piece type. The enhanced move generator
 * leverages piece lists, template functions, and SIMD-optimized algorithms to
 * achieve maximum performance in time-critical search operations.
 * 
 * ## Architecture
 * - **Modular Design**: Separate optimization modules for each piece type
 * - **Piece Lists**: Direct iteration over active pieces (fastest approach)
 * - **Template Specialization**: Compile-time optimization for piece types
 * - **SIMD Integration**: Hardware-accelerated bitboard operations
 * 
 * ## Performance Impact
 * Profiling-driven optimizations address specific bottlenecks:
 * - Pawn moves: 20.3% of generation time → PawnOptimizations module
 * - Sliding pieces: 45%+ of generation time → SlidingPieceOptimizations module
 * - Knight/King moves: KnightOptimizations, KingOptimizations modules
 * 
 * ## Function Categories
 * - **Production**: generate_all_moves() - main entry point for search
 * - **Legacy**: Individual piece functions for compatibility/testing
 * - **Debugging**: Move validation and generation analysis
 * 
 * @author MTDuke71
 * @version 1.2
 * @see movegen_enhanced.hpp for interface declarations
 */
#include "movegen_enhanced.hpp"
#include "position.hpp"
#include "move.hpp"
#include "attack_detection.hpp"  // For Huginn::SqAttacked function
#include "board120.hpp"
#include "chess_types.hpp"
#include "pawn_optimizations.hpp"
#include "king_optimizations.hpp"
#include "knight_optimizations.hpp"
#include "knight_lookup_tables.hpp"  // New lookup table optimization
#include "king_lookup_tables.hpp"    // King lookup table optimization
#include "sliding_piece_optimizations.hpp"
#include "bitboard_movegen.hpp"

/**
 * @brief Generate all legal moves for the current position
 * 
 * Main entry point for move generation during search. Uses optimized piece-specific
 * modules to generate moves efficiently, with each module targeting specific
 * performance bottlenecks identified through profiling.
 * 
 * @param pos Current chess position
 * @param list Move list to populate with generated moves
 * 
 * The function leverages:
 * - PawnOptimizations: Handles 20.3% of generation overhead
 * - SlidingPieceOptimizations: Handles 45%+ of generation overhead  
 * - Knight optimizations: Template-based or lookup table-based fast generation
 * - Direct piece list iteration: Fastest traversal method
 */
void generate_all_moves(const Position& pos, S_MOVELIST& list) {
    BitboardMoveGen::generate_all_moves_bitboard(pos, list);
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
    // Delegate to the optimized implementation
    #ifdef USE_KNIGHT_LOOKUP_TABLES
        KnightLookupTables::generate_knight_moves_lookup(pos, list, us);
    #else
        KnightOptimizations::generate_knight_moves_template(pos, list, us);
    #endif
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
            list.add_quiet_move(pseudo_moves[i]);  // Move is already legal
            temp_pos.TakeMove();
        }
    }
}

// VICE Part 65: Generate only capture moves for quiescence search
void generate_all_caps(Position& pos, S_MOVELIST& list) {
    S_MOVELIST all_moves;
    generate_all_moves(pos, all_moves);
    
    list.count = 0;  // Direct clear
    
    // Work on a copy to avoid modifying the input position
    Position temp_pos = pos;
    
    // Filter to only capture moves and check legality
    for (int i = 0; i < all_moves.size(); ++i) {
        S_MOVE move = all_moves[i];
        
        // Only include captures (including en passant)
        if (move.is_capture()) {
            if (temp_pos.MakeMove(move) == 1) {
                list.add_capture_move(move, pos);  // Legal capture move
                temp_pos.TakeMove();
            }
        }
    }
}

// =============================================================================
// BACKWARD COMPATIBILITY IMPLEMENTATIONS
// =============================================================================

void generate_king_moves(const Position& pos, S_MOVELIST& list, Color us) {
    // Delegate to the optimized implementation
    #ifdef USE_KING_LOOKUP_TABLES
        KingLookupTables::generate_king_moves_lookup(pos, list, us);
    #else
        KingOptimizations::generate_king_moves_optimized(pos, list, us);
    #endif
}
