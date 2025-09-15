/**
 * @file debug_position_issue.cpp
 * @brief Test the position after move sequence to debug book issue
 */

#include "position.hpp"
#include "uci.hpp"
#include <iostream>

int main() {
    std::cout << "=== Debugging Position Issue ===\n";
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "Starting position FEN: " << pos.to_fen() << "\n";
    
    // Apply the moves: c2c4 g8f6 d2d4 e7e6 g1f3 b7b6 b1c3 c8b7 a2a3 d7d5
    std::vector<std::string> moves = {"c2c4", "g8f6", "d2d4", "e7e6", "g1f3", "b7b6", "b1c3", "c8b7", "a2a3", "d7d5"};
    
    for (const auto& move_str : moves) {
        std::cout << "Applying move: " << move_str;
        
        // Parse UCI move
        S_MOVE move = parse_uci_move(move_str, pos);
        if (move.move == 0) {
            std::cout << " - FAILED to parse\n";
            break;
        }
        
        // Make the move
        if (pos.MakeMove(move) != 1) {
            std::cout << " - ILLEGAL move\n";
            break;
        }
        
        std::cout << " - SUCCESS\n";
        std::cout << "  New FEN: " << pos.to_fen() << "\n";
    }
    
    std::cout << "\n=== Final Position Analysis ===\n";
    std::cout << "Final FEN: " << pos.to_fen() << "\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    
    // Check if d7d5 is still a legal move
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    
    std::cout << "Legal moves count: " << legal_moves.count << "\n";
    std::cout << "Legal moves: ";
    for (int i = 0; i < legal_moves.count; ++i) {
        std::cout << move_to_uci(legal_moves.moves[i]) << " ";
    }
    std::cout << "\n";
    
    // Check specifically for d7d5
    bool d7d5_legal = false;
    for (int i = 0; i < legal_moves.count; ++i) {
        if (move_to_uci(legal_moves.moves[i]) == "d7d5") {
            d7d5_legal = true;
            break;
        }
    }
    
    std::cout << "Is d7d5 legal? " << (d7d5_legal ? "YES - This is the BUG!" : "NO - Correct") << "\n";
    
    return 0;
}