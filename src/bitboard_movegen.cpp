/**
 * @file bitboard_movegen.cpp  
 * @brief Implementation of true bitboard move generation
 * 
 * This demonstrates the actual bitboard approach that should show
 * dramatic performance improvements over piece lists.
 * 
 * @author MTDuke71
 * @version 1.0
 */

#include "bitboard_movegen.hpp"
#include "board120.hpp"
#include "king_lookup_tables.hpp"

namespace BitboardMoveGen {

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
    
    // Castling moves - this was missing!
    KingLookupTables::generate_castling_moves_optimized(pos, list, us);
}

void generate_knight_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us) {
    // Get knight bitboard - this is the key difference from piece lists!
    uint64_t knights = pos.piece_bitboards[int(us)][int(PieceType::Knight)];
    uint64_t own_pieces = pos.color_bitboards[int(us)];
    
    // Iterate through knights using bitboard operations
    while (knights != 0) {
        int from_sq64 = get_lsb(knights);
        knights &= knights - 1;  // Remove processed knight
        
        int from_sq120 = MAILBOX_MAPS.to120[from_sq64];
        
        // Get all knight attacks for this square - O(1) lookup!
        uint64_t attacks = KnightLookupTables::KNIGHT_ATTACKS[from_sq64];
        
        // Remove our own pieces from targets
        attacks &= ~own_pieces;
        
        // Generate moves for each target square
        while (attacks != 0) {
            int to_sq64 = get_lsb(attacks);
            attacks &= attacks - 1;  // Remove processed target
            
            int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
            
            // Check if it's a capture or quiet move
            Piece target = pos.at(to_sq120);
            if (target == Piece::None) {
                list.add_quiet_move(make_move(from_sq120, to_sq120));
            } else {
                list.add_capture_move(make_capture(from_sq120, to_sq120, type_of(target)), pos);
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
            
            int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
            int from_sq120 = to_sq120 - 10;  // One rank south
            
            // Check for promotion
            if (to_sq64 >= 56) {  // 8th rank
                list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Queen));
                list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Rook));
                list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Bishop));
                list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Knight));
            } else {
                list.add_quiet_move(make_move(from_sq120, to_sq120));
            }
        }
        
        // Generate double pushes
        pushes = double_pushes;
        while (pushes != 0) {
            int to_sq64 = get_lsb(pushes);
            pushes &= pushes - 1;

            int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
            int from_sq120 = to_sq120 - 20;  // Two ranks south

            list.add_quiet_move(make_pawn_start(from_sq120, to_sq120));
        }

        // White pawn captures using attack bitboards
        uint64_t pawn_copy = pos.piece_bitboards[int(us)][int(PieceType::Pawn)];
        while (pawn_copy != 0) {
            int from_sq64 = get_lsb(pawn_copy);
            pawn_copy &= pawn_copy - 1;
            
            int from_sq120 = MAILBOX_MAPS.to120[from_sq64];
            
            // Get pawn attacks and filter to enemy pieces
            uint64_t attacks = PawnLookupTables::get_pawn_attacks(Color::White, from_sq64) & enemies;
            
            while (attacks != 0) {
                int to_sq64 = get_lsb(attacks);
                attacks &= attacks - 1;
                
                int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
                Piece captured = pos.at(to_sq120);
                
                if (to_sq64 >= 56) {  // Promotion capture
                    list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Queen, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Rook, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Bishop, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Knight, type_of(captured)));
                } else {
                    list.add_capture_move(make_capture(from_sq120, to_sq120, type_of(captured)), pos);
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
            
            int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
            int from_sq120 = to_sq120 + 10;  // One rank north
            
            if (to_sq64 <= 7) {  // 1st rank promotion
                list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Queen));
                list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Rook));
                list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Bishop));
                list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Knight));
            } else {
                list.add_quiet_move(make_move(from_sq120, to_sq120));
            }
        }
        
        // Generate double pushes
        pushes = double_pushes;
        while (pushes != 0) {
            int to_sq64 = get_lsb(pushes);
            pushes &= pushes - 1;

            int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
            int from_sq120 = to_sq120 + 20;  // Two ranks north

            list.add_quiet_move(make_pawn_start(from_sq120, to_sq120));
        }
        
        // Black pawn captures
        uint64_t pawn_copy = pos.piece_bitboards[int(us)][int(PieceType::Pawn)];
        while (pawn_copy != 0) {
            int from_sq64 = get_lsb(pawn_copy);
            pawn_copy &= pawn_copy - 1;
            
            int from_sq120 = MAILBOX_MAPS.to120[from_sq64];
            
            uint64_t attacks = PawnLookupTables::get_pawn_attacks(Color::Black, from_sq64) & enemies;
            
            while (attacks != 0) {
                int to_sq64 = get_lsb(attacks);
                attacks &= attacks - 1;
                
                int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
                Piece captured = pos.at(to_sq120);
                
                if (to_sq64 <= 7) {  // Promotion capture
                    list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Queen, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Rook, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Bishop, type_of(captured)));
                    list.add_promotion_move(make_promotion(from_sq120, to_sq120, PieceType::Knight, type_of(captured)));
                } else {
                    list.add_capture_move(make_capture(from_sq120, to_sq120, type_of(captured)), pos);
                }
            }
        }
    }
    
    // Generate en passant captures
    if (pos.ep_square != -1) {
        int ep_sq120 = pos.ep_square;
        int ep_sq64 = MAILBOX_MAPS.to64[ep_sq120];
        
        if (ep_sq64 >= 0) {  // Valid 64-square position
            uint64_t ep_attackers = 0;
            
            if (us == Color::White) {
                // White pawns can capture en passant if they're on rank 5 (sq64: 32-39)
                // and can attack the en passant square
                ep_attackers = PawnLookupTables::get_pawn_attacks(Color::Black, ep_sq64) & pawns;
            } else {
                // Black pawns can capture en passant if they're on rank 4 (sq64: 24-31) 
                // and can attack the en passant square
                ep_attackers = PawnLookupTables::get_pawn_attacks(Color::White, ep_sq64) & pawns;
            }
            
            while (ep_attackers != 0) {
                int from_sq64 = get_lsb(ep_attackers);
                ep_attackers &= ep_attackers - 1;
                
                int from_sq120 = MAILBOX_MAPS.to120[from_sq64];
                list.add_en_passant_move(make_en_passant(from_sq120, ep_sq120));
            }
        }
    }
}

void generate_king_moves_bitboard(const Position& pos, S_MOVELIST& list, Color us) {
    uint64_t king = pos.piece_bitboards[int(us)][int(PieceType::King)];
    if (king == 0) return;
    
    int king_sq64 = get_lsb(king);
    int king_sq120 = MAILBOX_MAPS.to120[king_sq64];
    
    // Get king attacks - O(1) lookup
    uint64_t attacks = KingLookupTables::KING_ATTACKS[king_sq64];
    uint64_t own_pieces = pos.color_bitboards[int(us)];
    
    // Remove own pieces
    attacks &= ~own_pieces;
    
    while (attacks != 0) {
        int to_sq64 = get_lsb(attacks);
        attacks &= attacks - 1;
        
        int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
        Piece target = pos.at(to_sq120);
        
        if (target == Piece::None) {
            list.add_quiet_move(make_move(king_sq120, to_sq120));
        } else {
            list.add_capture_move(make_capture(king_sq120, to_sq120, type_of(target)), pos);
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
        
        int from_sq120 = MAILBOX_MAPS.to120[from_sq64];
        
        // Get bishop attacks with occupancy
        uint64_t attacks = bishop_attacks(from_sq64, occupied) & ~own_pieces;
        
        while (attacks != 0) {
            int to_sq64 = get_lsb(attacks);
            attacks &= attacks - 1;
            
            int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
            Piece target = pos.at(to_sq120);
            
            if (target == Piece::None) {
                list.add_quiet_move(make_move(from_sq120, to_sq120));
            } else {
                list.add_capture_move(make_capture(from_sq120, to_sq120, type_of(target)), pos);
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
        
        int from_sq120 = MAILBOX_MAPS.to120[from_sq64];
        
        // Get rook attacks with occupancy
        uint64_t attacks = rook_attacks(from_sq64, occupied) & ~own_pieces;
        
        while (attacks != 0) {
            int to_sq64 = get_lsb(attacks);
            attacks &= attacks - 1;
            
            int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
            Piece target = pos.at(to_sq120);
            
            if (target == Piece::None) {
                list.add_quiet_move(make_move(from_sq120, to_sq120));
            } else {
                list.add_capture_move(make_capture(from_sq120, to_sq120, type_of(target)), pos);
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
        
        int from_sq120 = MAILBOX_MAPS.to120[from_sq64];
        
        // Get queen attacks (bishop + rook) with occupancy
        uint64_t attacks = queen_attacks(from_sq64, occupied) & ~own_pieces;
        
        while (attacks != 0) {
            int to_sq64 = get_lsb(attacks);
            attacks &= attacks - 1;
            
            int to_sq120 = MAILBOX_MAPS.to120[to_sq64];
            Piece target = pos.at(to_sq120);
            
            if (target == Piece::None) {
                list.add_quiet_move(make_move(from_sq120, to_sq120));
            } else {
                list.add_capture_move(make_capture(from_sq120, to_sq120, type_of(target)), pos);
            }
        }
    }
}

} // namespace BitboardMoveGen