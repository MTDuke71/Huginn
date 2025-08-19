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

// Test to examine the position after a2a4 specifically
TEST(PerftDebug, ExamineA2A4Position) {
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    std::cout << "=== Examining position after a2a4 ===\n";
    
    MoveList list;
    generate_legal_moves(pos, list);
    
    for (const auto& m : list.v) {
        if (move_to_string(m) == "a2a4") {
            State st{};
            make_move(pos, m, st);
            
            // Print the resulting FEN
            std::cout << "FEN after a2a4: " << pos.to_fen() << "\n";
            
            // Generate Black's response moves
            MoveList response_list;
            generate_legal_moves(pos, response_list);
            
            std::cout << "Black has " << response_list.v.size() << " moves (expected: 44):\n";
            
            // Print all moves sorted
            std::vector<std::string> move_strings;
            for (const auto& response_move : response_list.v) {
                move_strings.push_back(move_to_string(response_move));
            }
            std::sort(move_strings.begin(), move_strings.end());
            
            for (size_t i = 0; i < move_strings.size(); ++i) {
                std::cout << move_strings[i];
                if (i < move_strings.size() - 1) std::cout << " ";
                if ((i + 1) % 10 == 0) std::cout << "\n";
            }
            std::cout << "\n";
            
            std::cout << "Total moves: " << response_list.v.size() << "\n";
            std::cout << "Expected: 44\n";
            std::cout << "Difference: " << (static_cast<int>(response_list.v.size()) - 44) << "\n";
            
            // Check en passant square
            std::cout << "En passant square: " << pos.ep_square << "\n";
            
            unmake_move(pos, m, st);
            break;
        }
    }
}
