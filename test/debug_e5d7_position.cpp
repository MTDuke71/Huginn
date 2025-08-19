#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen.hpp"
#include "sq_attacked.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <set>

// Convert S_MOVE to algebraic notation for debugging
static std::string move_to_string(const S_MOVE& move) {
    int from = move.get_from();
    int to = move.get_to();
    
    std::string result;
    result += char('a' + int(file_of(from)));
    result += char('1' + int(rank_of(from)));
    result += char('a' + int(file_of(to)));
    result += char('1' + int(rank_of(to)));
    
    if (move.is_promotion()) {
        switch (move.get_promoted()) {
            case PieceType::Queen: result += 'q'; break;
            case PieceType::Rook: result += 'r'; break;
            case PieceType::Bishop: result += 'b'; break;
            case PieceType::Knight: result += 'n'; break;
            default: break;
        }
    }
    
    return result;
}

// Test to examine the position after e5d7 specifically
TEST(PerftDebug, ExamineE5D7Position) {
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    std::cout << "=== Examining position after e5d7 ===\n";
    
    MoveList list;
    generate_legal_moves(pos, list);
    
    for (const auto& m : list.v) {
        if (move_to_string(m) == "e5d7") {
            State st{};
            make_move(pos, m, st);
            
            // Print the resulting FEN
            std::cout << "FEN after e5d7: " << pos.to_fen() << "\n";
            
            // Generate Black's response moves
            MoveList response_list;
            generate_legal_moves(pos, response_list);
            
            std::cout << "Black has " << response_list.v.size() << " moves:\n";
            
            // Group moves by piece type for analysis
            std::vector<std::string> pawn_moves, piece_moves, king_moves, castling_moves;
            std::set<std::string> all_moves;
            
            for (const auto& response_move : response_list.v) {
                std::string move_str = move_to_string(response_move);
                
                // Check for duplicates
                if (all_moves.count(move_str)) {
                    std::cout << "DUPLICATE MOVE FOUND: " << move_str << "\n";
                } else {
                    all_moves.insert(move_str);
                }
                
                Piece moving_piece = pos.board[response_move.get_from()];
                
                if (response_move.is_castle()) {
                    castling_moves.push_back(move_str);
                } else if (type_of(moving_piece) == PieceType::Pawn) {
                    pawn_moves.push_back(move_str);
                } else if (type_of(moving_piece) == PieceType::King) {
                    king_moves.push_back(move_str);
                } else {
                    piece_moves.push_back(move_str);
                }
            }
            
            std::cout << "\nMove breakdown:\n";
            std::cout << "Pawn moves (" << pawn_moves.size() << "): ";
            for (const auto& move : pawn_moves) std::cout << move << " ";
            std::cout << "\n";
            
            std::cout << "Piece moves (" << piece_moves.size() << "): ";
            for (const auto& move : piece_moves) std::cout << move << " ";
            std::cout << "\n";
            
            std::cout << "King moves (" << king_moves.size() << "): ";
            for (const auto& move : king_moves) std::cout << move << " ";
            std::cout << "\n";
            
            std::cout << "Castling moves (" << castling_moves.size() << "): ";
            for (const auto& move : castling_moves) std::cout << move << " ";
            std::cout << "\n";
            
            // Check for any suspicious patterns
            std::cout << "\nChecking for potential issues:\n";
            
            // Check if there are any illegal moves (king left in check)
            int illegal_count = 0;
            for (const auto& response_move : response_list.v) {
                State st2{};
                make_move(pos, response_move, st2);
                
                Color moving_side = !pos.side_to_move;
                int king_sq = pos.king_sq[static_cast<int>(moving_side)];
                bool king_in_check = SqAttacked(king_sq, pos, !moving_side);
                
                if (king_in_check) {
                    std::cout << "ILLEGAL MOVE: " << move_to_string(response_move) << " leaves king in check\n";
                    illegal_count++;
                }
                
                unmake_move(pos, response_move, st2);
            }
            
            if (illegal_count == 0) {
                std::cout << "All moves are legal.\n";
            }
            
            std::cout << "Total unique moves: " << all_moves.size() << "\n";
            std::cout << "Expected for this position: 44 (based on standard perft)\n";
            std::cout << "Difference: " << (static_cast<int>(all_moves.size()) - 44) << "\n";
            
            unmake_move(pos, m, st);
            break;
        }
    }
}
