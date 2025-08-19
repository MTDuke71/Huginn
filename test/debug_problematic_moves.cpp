#include <gtest/gtest.h>
#include "../src/position.h"
#include "../src/movegen_simple.h"
#include "../src/move.h"
#include <iostream>
#include <vector>
#include <string>

// Test to examine the specific moves that generate extra perft counts
TEST(PerftDebug, ExamineProblematicMoves) {
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    std::vector<std::string> problematic_moves = {"d5e6", "e5d7", "e5f7", "f3f5"};
    
    for (const std::string& move_str : problematic_moves) {
        std::cout << "\n=== Examining move: " << move_str << " ===\n";
        
        MoveList list;
        generate_legal_moves(pos, list);
        
        for (const auto& m : list.v) {
            if (move_to_string(m) == move_str) {
                State st{};
                make_move(pos, m, st);
                
                // Check material count
                int white_material = 0, black_material = 0;
                for (int sq = 0; sq < 64; sq++) {
                    Piece p = pos.board[sq];
                    if (p != PIECE_EMPTY) {
                        int piece_value = 1; // Just count pieces, not values
                        if (piece_color(p) == WHITE) {
                            white_material += piece_value;
                        } else {
                            black_material += piece_value;
                        }
                    }
                }
                
                // Check castling rights
                std::cout << "Castling rights: ";
                if (pos.castling_rights & CASTLE_WK) std::cout << "K";
                if (pos.castling_rights & CASTLE_WQ) std::cout << "Q";
                if (pos.castling_rights & CASTLE_BK) std::cout << "k";
                if (pos.castling_rights & CASTLE_BQ) std::cout << "q";
                if (pos.castling_rights == 0) std::cout << "-";
                std::cout << "\n";
                
                std::cout << "White material: " << white_material << ", Black material: " << black_material << "\n";
                std::cout << "En passant square: " << pos.ep_sq << "\n";
                std::cout << "Side to move: " << (pos.side_to_move == WHITE ? "Black" : "White") << "\n";
                
                // Generate moves and categorize them
                MoveList response_list;
                generate_legal_moves(pos, response_list);
                
                int pawn_moves = 0, piece_moves = 0, king_moves = 0, castling_moves = 0, captures = 0, ep_captures = 0;
                
                for (const auto& response_move : response_list.v) {
                    Piece moving_piece = pos.board[response_move.from()];
                    Piece captured_piece = pos.board[response_move.to()];
                    
                    if (response_move.is_castle()) {
                        castling_moves++;
                    } else if (piece_type(moving_piece) == PAWN) {
                        pawn_moves++;
                        if (response_move.is_ep()) ep_captures++;
                        if (captured_piece != PIECE_EMPTY) captures++;
                    } else if (piece_type(moving_piece) == KING) {
                        king_moves++;
                        if (captured_piece != PIECE_EMPTY) captures++;
                    } else {
                        piece_moves++;
                        if (captured_piece != PIECE_EMPTY) captures++;
                    }
                }
                
                std::cout << "Move breakdown:\n";
                std::cout << "  Pawn moves: " << pawn_moves << "\n";
                std::cout << "  Piece moves: " << piece_moves << "\n";
                std::cout << "  King moves: " << king_moves << "\n";
                std::cout << "  Castling moves: " << castling_moves << "\n";
                std::cout << "  Captures: " << captures << "\n";
                std::cout << "  En passant captures: " << ep_captures << "\n";
                std::cout << "  Total: " << response_list.v.size() << "\n";
                
                unmake_move(pos, m, st);
                break;
            }
        }
    }
}
