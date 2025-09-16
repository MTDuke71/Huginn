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

void BitboardMoveGen::generate_legal_moves(BitboardPosition& pos, BitboardMoveList& moves) {
    // First generate all pseudo-legal moves
    BitboardMoveList pseudo_legal_moves;
    generate_all_moves(pos, pseudo_legal_moves);
    
    // Filter out illegal moves (those that leave king in check)
    moves.clear();
    for (size_t i = 0; i < pseudo_legal_moves.moves.size(); i++) {
        // Convert to SimpleBitboardMove for legality testing
        SimpleBitboardMove simple_move;
        simple_move.from_64 = pseudo_legal_moves.moves[i].from_64;
        simple_move.to_64 = pseudo_legal_moves.moves[i].to_64;
        simple_move.is_capture = pseudo_legal_moves.moves[i].is_capture;
        simple_move.is_ep_capture = pseudo_legal_moves.moves[i].is_ep_capture;
        simple_move.is_castling = pseudo_legal_moves.moves[i].is_castling;
        simple_move.is_promotion = pseudo_legal_moves.moves[i].is_promotion;
        simple_move.promotion_type = pseudo_legal_moves.moves[i].promotion_type;
        
        if (pos.is_legal_move(simple_move)) {
            moves.moves.push_back(pseudo_legal_moves.moves[i]);
        }
    }
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
    
    // ---- En passant captures ----
    if (pos.ep_square_64 != -1) {
        int ep_square = pos.ep_square_64;
        
        // Check for white pawns that can capture en passant
        // En passant target square is on rank 6 for white pawns
        if (ep_square >= 40 && ep_square <= 47) { // rank 6 (0-indexed)
            // For white pawns, they attack diagonally up-left and up-right
            // If en passant square is the target, check the squares diagonally down-left and down-right
            
            // Check diagonal capture from down-left (a5 -> b6 type)
            if ((ep_square % 8) > 0) { // not on a-file
                int pawn_square = ep_square - 8 - 1; // one rank down, one file left
                // BOARD WRAP FIX: Ensure the pawn is actually on an adjacent file
                if (pawn_square >= 0 && (pawn_square % 8) == ((ep_square % 8) - 1)) {
                    if (pos.piece_at(pawn_square) == Piece::WhitePawn) {
                        BitboardMoveList::BitboardMove move(pawn_square, ep_square);
                        move.is_capture = true;
                        move.is_ep_capture = true;
                        moves.moves.push_back(move);
                    }
                }
            }
            
            // Check diagonal capture from down-right (c5 -> b6 type)  
            if ((ep_square % 8) < 7) { // not on h-file
                int pawn_square = ep_square - 8 + 1; // one rank down, one file right
                // BOARD WRAP FIX: Ensure the pawn is actually on an adjacent file
                if (pawn_square >= 0 && (pawn_square % 8) == ((ep_square % 8) + 1)) {
                    if (pos.piece_at(pawn_square) == Piece::WhitePawn) {
                        BitboardMoveList::BitboardMove move(pawn_square, ep_square);
                        move.is_capture = true;
                        move.is_ep_capture = true;
                        moves.moves.push_back(move);
                    }
                }
            }
        }
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
    
    // ---- En passant captures ----
    if (pos.ep_square_64 != -1) {
        int ep_square = pos.ep_square_64;
        
        // Check for black pawns that can capture en passant
        // En passant target square is on rank 3 for black pawns
        if (ep_square >= 16 && ep_square <= 23) { // rank 3 (0-indexed)
            // For black pawns, they attack diagonally down-left and down-right
            // If en passant square is the target, check the squares diagonally up-left and up-right
            
            // Check diagonal capture from up-left (a4 -> b3 type)
            if ((ep_square % 8) > 0) { // not on a-file
                int pawn_square = ep_square + 8 - 1; // one rank up, one file left
                // BOARD WRAP FIX: Ensure the pawn is actually on an adjacent file
                if (pawn_square <= 63 && (pawn_square % 8) == ((ep_square % 8) - 1)) {
                    if (pos.piece_at(pawn_square) == Piece::BlackPawn) {
                        BitboardMoveList::BitboardMove move(pawn_square, ep_square);
                        move.is_capture = true;
                        move.is_ep_capture = true;
                        moves.moves.push_back(move);
                    }
                }
            }
            
            // Check diagonal capture from up-right (c4 -> b3 type)  
            if ((ep_square % 8) < 7) { // not on h-file
                int pawn_square = ep_square + 8 + 1; // one rank up, one file right
                // BOARD WRAP FIX: Ensure the pawn is actually on an adjacent file
                if (pawn_square <= 63 && (pawn_square % 8) == ((ep_square % 8) + 1)) {
                    if (pos.piece_at(pawn_square) == Piece::BlackPawn) {
                        BitboardMoveList::BitboardMove move(pawn_square, ep_square);
                        move.is_capture = true;
                        move.is_ep_capture = true;
                        moves.moves.push_back(move);
                    }
                }
            }
        }
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
        
        // King cannot move to attacked squares
        if (pos.is_square_attacked(to_64, static_cast<Color>(1 - static_cast<int>(us)))) {
            continue; // Skip this destination as it's attacked by enemy
        }
        
        if (pos.is_square_occupied(to_64)) {
            // Capture move
            moves.add_capture(king_square, to_64);
        } else {
            // Quiet move
            moves.add_move(king_square, to_64);
        }
    }
    
    // Generate castling moves
    if (pos.side_to_move == Color::White) {
        // White castling
        // Kingside castling (e1g1)
        if ((pos.castling_rights & 1) && // White kingside right available
            !pos.is_square_occupied(5) && !pos.is_square_occupied(6) && // f1 and g1 empty
            !pos.is_square_attacked(4, Color::Black) &&  // King not in check
            !pos.is_square_attacked(5, Color::Black) &&  // King doesn't pass through check (f1)
            !pos.is_square_attacked(6, Color::Black)) {  // King doesn't end in check (g1)
            BitboardMoveList::BitboardMove castle_move(4, 6); // e1 to g1
            castle_move.is_castling = true;
            moves.moves.push_back(castle_move);
        }
        
        // Queenside castling (e1c1)  
        if ((pos.castling_rights & 2) && // White queenside right available
            !pos.is_square_occupied(1) && !pos.is_square_occupied(2) && !pos.is_square_occupied(3) && // b1, c1, d1 empty
            !pos.is_square_attacked(4, Color::Black) &&  // King not in check
            !pos.is_square_attacked(3, Color::Black) &&  // King doesn't pass through check (d1)
            !pos.is_square_attacked(2, Color::Black)) {  // King doesn't end in check (c1)
            BitboardMoveList::BitboardMove castle_move(4, 2); // e1 to c1
            castle_move.is_castling = true;
            moves.moves.push_back(castle_move);
        }
    } else {
        // Black castling
        // Kingside castling (e8g8)
        if ((pos.castling_rights & 4) && // Black kingside right available
            !pos.is_square_occupied(61) && !pos.is_square_occupied(62) && // f8 and g8 empty
            !pos.is_square_attacked(60, Color::White) && // King not in check
            !pos.is_square_attacked(61, Color::White) && // King doesn't pass through check (f8)
            !pos.is_square_attacked(62, Color::White)) { // King doesn't end in check (g8)
            BitboardMoveList::BitboardMove castle_move(60, 62); // e8 to g8
            castle_move.is_castling = true;
            moves.moves.push_back(castle_move);
        }
        
        // Queenside castling (e8c8)
        if ((pos.castling_rights & 8) && // Black queenside right available
            !pos.is_square_occupied(57) && !pos.is_square_occupied(58) && !pos.is_square_occupied(59) && // b8, c8, d8 empty
            !pos.is_square_attacked(60, Color::White) && // King not in check
            !pos.is_square_attacked(59, Color::White) && // King doesn't pass through check (d8)
            !pos.is_square_attacked(58, Color::White)) { // King doesn't end in check (c8)
            BitboardMoveList::BitboardMove castle_move(60, 58); // e8 to c8
            castle_move.is_castling = true;
            moves.moves.push_back(castle_move);
        }
    }
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