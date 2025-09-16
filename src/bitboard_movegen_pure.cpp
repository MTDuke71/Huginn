/**
 * @file bitboard_movegen_pure.cpp
 * @brief Implementation of native bitboard move generation
 * 
 * Pure 64-square bitboard move generation with zero conversion overhead.
 * Demonstrates true bitboard performance potential without mailbox dependencies.
 * 
 * @author MTDuke71
 * @version 1.0
 */

#include "bitboard_movegen_pure.hpp"
#include "bitboard.hpp"  // For pop_lsb, setBit utilities
#include "bitboard_attacks.hpp"  // For precomputed attack tables

// ============================================================================
// MAIN MOVE GENERATION ENTRY POINT
// ============================================================================

void BitboardMoveGen::generate_all_moves(const BitboardPosition& pos, BitboardMoveList& moves) {
    moves.clear();
    
    generate_pawn_moves(pos, moves);
    generate_knight_moves(pos, moves);
    generate_bishop_moves(pos, moves);
    generate_rook_moves(pos, moves);
    generate_queen_moves(pos, moves);
    generate_king_moves(pos, moves);
}

// ============================================================================
// OPTIMIZED PAWN MOVE GENERATION (PURE BITBOARD)
// ============================================================================

void BitboardMoveGen::generate_pawn_moves(const BitboardPosition& pos, BitboardMoveList& moves) {
    if (pos.side_to_move == Color::White) {
        generate_white_pawn_moves_optimized(pos, moves);
    } else {
        generate_black_pawn_moves_optimized(pos, moves);
    }
}

void BitboardMoveGen::generate_white_pawn_moves_optimized(const BitboardPosition& pos, BitboardMoveList& moves) {
    uint64_t our_pawns = pos.get_pieces(Color::White, PieceType::Pawn);
    uint64_t empty = pos.get_empty();
    uint64_t enemies = pos.get_all_pieces(Color::Black);
    
    // ---- Single pawn pushes (pure bitboard operation) ----
    uint64_t single_pushes = (our_pawns << 8) & empty;
    
    // ---- Double pawn pushes (chained bitboard operation) ----
    uint64_t double_pushes = ((single_pushes & RANK_3_BB) << 8) & empty;
    
    // ---- Captures (bitboard with file mask) ----
    uint64_t left_captures = (our_pawns << 7) & enemies & ~FILE_H_BB;
    uint64_t right_captures = (our_pawns << 9) & enemies & ~FILE_A_BB;
    
    // ---- Separate promotions from regular moves ----
    uint64_t promotion_pushes = single_pushes & RANK_8_BB;
    uint64_t promotion_left_captures = left_captures & RANK_8_BB;
    uint64_t promotion_right_captures = right_captures & RANK_8_BB;
    
    // ---- Regular moves (subtract promotions) ----
    single_pushes &= ~RANK_8_BB;
    left_captures &= ~RANK_8_BB;
    right_captures &= ~RANK_8_BB;
    
    // ---- Process moves efficiently ----
    
    // Single pushes
    while (single_pushes != 0) {
        int to_64 = pop_lsb(single_pushes);
        int from_64 = to_64 - 8;
        moves.add_move(from_64, to_64);
    }
    
    // Double pushes
    while (double_pushes != 0) {
        int to_64 = pop_lsb(double_pushes);
        int from_64 = to_64 - 16;
        moves.add_move(from_64, to_64);
    }
    
    // Left captures
    while (left_captures != 0) {
        int to_64 = pop_lsb(left_captures);
        int from_64 = to_64 - 7;
        moves.add_capture(from_64, to_64);
    }
    
    // Right captures  
    while (right_captures != 0) {
        int to_64 = pop_lsb(right_captures);
        int from_64 = to_64 - 9;
        moves.add_capture(from_64, to_64);
    }
    
    // ---- Process promotions (all 4 types) ----
    while (promotion_pushes != 0) {
        int to_64 = pop_lsb(promotion_pushes);
        int from_64 = to_64 - 8;
        moves.add_move(from_64, to_64, PieceType::Queen);
        moves.add_move(from_64, to_64, PieceType::Rook);
        moves.add_move(from_64, to_64, PieceType::Bishop);
        moves.add_move(from_64, to_64, PieceType::Knight);
    }
    
    while (promotion_left_captures != 0) {
        int to_64 = pop_lsb(promotion_left_captures);
        int from_64 = to_64 - 7;
        moves.add_capture(from_64, to_64, PieceType::Queen);
        moves.add_capture(from_64, to_64, PieceType::Rook);
        moves.add_capture(from_64, to_64, PieceType::Bishop);
        moves.add_capture(from_64, to_64, PieceType::Knight);
    }
    
    while (promotion_right_captures != 0) {
        int to_64 = pop_lsb(promotion_right_captures);
        int from_64 = to_64 - 9;
        moves.add_capture(from_64, to_64, PieceType::Queen);
        moves.add_capture(from_64, to_64, PieceType::Rook);
        moves.add_capture(from_64, to_64, PieceType::Bishop);
        moves.add_capture(from_64, to_64, PieceType::Knight);
    }
}

void BitboardMoveGen::generate_black_pawn_moves_optimized(const BitboardPosition& pos, BitboardMoveList& moves) {
    uint64_t our_pawns = pos.get_pieces(Color::Black, PieceType::Pawn);
    uint64_t empty = pos.get_empty();
    uint64_t enemies = pos.get_all_pieces(Color::White);
    
    // ---- Single pawn pushes (pure bitboard operation) ----
    uint64_t single_pushes = (our_pawns >> 8) & empty;
    
    // ---- Double pawn pushes (chained bitboard operation) ----
    uint64_t double_pushes = ((single_pushes & RANK_6_BB) >> 8) & empty;
    
    // ---- Captures (bitboard with file mask) ----
    uint64_t left_captures = (our_pawns >> 9) & enemies & ~FILE_H_BB;
    uint64_t right_captures = (our_pawns >> 7) & enemies & ~FILE_A_BB;
    
    // ---- Separate promotions from regular moves ----
    uint64_t promotion_pushes = single_pushes & RANK_1_BB;
    uint64_t promotion_left_captures = left_captures & RANK_1_BB;
    uint64_t promotion_right_captures = right_captures & RANK_1_BB;
    
    // ---- Regular moves (subtract promotions) ----
    single_pushes &= ~RANK_1_BB;
    left_captures &= ~RANK_1_BB;
    right_captures &= ~RANK_1_BB;
    
    // ---- Process moves efficiently ----
    
    // Single pushes
    while (single_pushes != 0) {
        int to_64 = pop_lsb(single_pushes);
        int from_64 = to_64 + 8;
        moves.add_move(from_64, to_64);
    }
    
    // Double pushes
    while (double_pushes != 0) {
        int to_64 = pop_lsb(double_pushes);
        int from_64 = to_64 + 16;
        moves.add_move(from_64, to_64);
    }
    
    // Left captures
    while (left_captures != 0) {
        int to_64 = pop_lsb(left_captures);
        int from_64 = to_64 + 9;
        moves.add_capture(from_64, to_64);
    }
    
    // Right captures  
    while (right_captures != 0) {
        int to_64 = pop_lsb(right_captures);
        int from_64 = to_64 + 7;
        moves.add_capture(from_64, to_64);
    }
    
    // ---- Process promotions (all 4 types) ----
    while (promotion_pushes != 0) {
        int to_64 = pop_lsb(promotion_pushes);
        int from_64 = to_64 + 8;
        moves.add_move(from_64, to_64, PieceType::Queen);
        moves.add_move(from_64, to_64, PieceType::Rook);
        moves.add_move(from_64, to_64, PieceType::Bishop);
        moves.add_move(from_64, to_64, PieceType::Knight);
    }
    
    while (promotion_left_captures != 0) {
        int to_64 = pop_lsb(promotion_left_captures);
        int from_64 = to_64 + 9;
        moves.add_capture(from_64, to_64, PieceType::Queen);
        moves.add_capture(from_64, to_64, PieceType::Rook);
        moves.add_capture(from_64, to_64, PieceType::Bishop);
        moves.add_capture(from_64, to_64, PieceType::Knight);
    }
    
    while (promotion_right_captures != 0) {
        int to_64 = pop_lsb(promotion_right_captures);
        int from_64 = to_64 + 7;
        moves.add_capture(from_64, to_64, PieceType::Queen);
        moves.add_capture(from_64, to_64, PieceType::Rook);
        moves.add_capture(from_64, to_64, PieceType::Bishop);
        moves.add_capture(from_64, to_64, PieceType::Knight);
    }
}

// ============================================================================
// OTHER PIECE MOVE GENERATION (PLACEHOLDER IMPLEMENTATIONS)
// ============================================================================

void BitboardMoveGen::generate_knight_moves(const BitboardPosition& pos, BitboardMoveList& moves) {
    uint64_t our_knights = pos.get_pieces(pos.side_to_move, PieceType::Knight);
    uint64_t our_pieces = pos.get_all_pieces(pos.side_to_move);
    
    while (our_knights != 0) {
        int from_64 = pop_lsb(our_knights);
        
        // Get precomputed knight attacks for this square
        uint64_t attacks = get_knight_attacks(from_64);
        
        // Remove squares occupied by our own pieces
        attacks &= ~our_pieces;
        
        // Process all valid destination squares
        while (attacks != 0) {
            int to_64 = pop_lsb(attacks);
            
            if (pos.is_square_occupied(to_64)) {
                // Capture move
                moves.add_capture(from_64, to_64);
            } else {
                // Quiet move
                moves.add_move(from_64, to_64);
            }
        }
    }
}

void BitboardMoveGen::generate_bishop_moves(const BitboardPosition& pos, BitboardMoveList& moves) {
    Color us = pos.side_to_move;
    uint64_t our_bishops = pos.get_pieces(us, PieceType::Bishop);
    uint64_t occupied = pos.get_occupied();
    uint64_t our_pieces = pos.get_all_pieces(us);
    uint64_t enemy_pieces = pos.get_all_pieces(us == Color::White ? Color::Black : Color::White);
    
    // Process each bishop
    while (our_bishops != 0) {
        int from = pop_lsb(our_bishops);
        
        // Generate bishop attacks using existing bitboard function
        uint64_t attacks = bishop_attacks(from, occupied);
        
        // Remove squares occupied by our pieces
        attacks &= ~our_pieces;
        
        // Separate captures from quiet moves
        uint64_t captures = attacks & enemy_pieces;
        uint64_t quiet_moves = attacks & ~enemy_pieces;
        
        // Add moves with proper capture flags
        add_moves_from_bitboard(captures, from, moves, true);
        add_moves_from_bitboard(quiet_moves, from, moves, false);
    }
}

void BitboardMoveGen::generate_rook_moves(const BitboardPosition& pos, BitboardMoveList& moves) {
    Color us = pos.side_to_move;
    uint64_t our_rooks = pos.get_pieces(us, PieceType::Rook);
    uint64_t occupied = pos.get_occupied();
    uint64_t our_pieces = pos.get_all_pieces(us);
    uint64_t enemy_pieces = pos.get_all_pieces(us == Color::White ? Color::Black : Color::White);
    
    // Process each rook
    while (our_rooks != 0) {
        int from = pop_lsb(our_rooks);
        
        // Generate rook attacks using existing bitboard function
        uint64_t attacks = rook_attacks(from, occupied);
        
        // Remove squares occupied by our pieces
        attacks &= ~our_pieces;
        
        // Separate captures from quiet moves
        uint64_t captures = attacks & enemy_pieces;
        uint64_t quiet_moves = attacks & ~enemy_pieces;
        
        // Add moves with proper capture flags
        add_moves_from_bitboard(captures, from, moves, true);
        add_moves_from_bitboard(quiet_moves, from, moves, false);
    }
}

void BitboardMoveGen::generate_queen_moves(const BitboardPosition& pos, BitboardMoveList& moves) {
    Color us = pos.side_to_move;
    uint64_t our_queens = pos.get_pieces(us, PieceType::Queen);
    uint64_t occupied = pos.get_occupied();
    uint64_t our_pieces = pos.get_all_pieces(us);
    uint64_t enemy_pieces = pos.get_all_pieces(us == Color::White ? Color::Black : Color::White);
    
    // Process each queen
    while (our_queens != 0) {
        int from = pop_lsb(our_queens);
        
        // Generate queen attacks (combination of bishop and rook attacks)
        uint64_t attacks = bishop_attacks(from, occupied) | rook_attacks(from, occupied);
        
        // Remove squares occupied by our pieces
        attacks &= ~our_pieces;
        
        // Separate captures from quiet moves
        uint64_t captures = attacks & enemy_pieces;
        uint64_t quiet_moves = attacks & ~enemy_pieces;
        
        // Add moves with proper capture flags
        add_moves_from_bitboard(captures, from, moves, true);
        add_moves_from_bitboard(quiet_moves, from, moves, false);
    }
}

void BitboardMoveGen::generate_king_moves(const BitboardPosition& pos, BitboardMoveList& moves) {
    Color us = pos.side_to_move;
    int king_square = pos.king_square_64[static_cast<int>(us)];
    
    if (king_square == -1) return;  // No king found
    
    uint64_t our_pieces = pos.get_all_pieces(us);
    
    // Get precomputed king attacks for this square
    uint64_t attacks = get_king_attacks(king_square);
    
    // Remove squares occupied by our own pieces
    attacks &= ~our_pieces;
    
    // Process all valid destination squares
    while (attacks != 0) {
        int to_64 = pop_lsb(attacks);
        
        if (pos.is_square_occupied(to_64)) {
            // Capture move
            moves.add_capture(king_square, to_64);
        } else {
            // Quiet move
            moves.add_move(king_square, to_64);
        }
    }
    
    // TODO: Add castling moves when castling rights are implemented
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void BitboardMoveGen::add_moves_from_bitboard(uint64_t move_bitboard, int from_square, 
                                             BitboardMoveList& moves, bool is_capture) {
    while (move_bitboard != 0) {
        int to_square = pop_lsb(move_bitboard);
        if (is_capture) {
            moves.add_capture(from_square, to_square);
        } else {
            moves.add_move(from_square, to_square);
        }
    }
}

void BitboardMoveGen::add_promotion_moves_from_bitboard(uint64_t promo_bitboard, int from_square,
                                                       BitboardMoveList& moves, bool is_capture) {
    while (promo_bitboard != 0) {
        int to_square = pop_lsb(promo_bitboard);
        
        // Add all 4 promotion types
        PieceType promotions[] = {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight};
        for (PieceType promo : promotions) {
            if (is_capture) {
                moves.add_capture(from_square, to_square, promo);
            } else {
                moves.add_move(from_square, to_square, promo);
            }
        }
    }
}