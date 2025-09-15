/**
 * @file debug_side_to_move.cpp
 * @brief Debug the side-to-move issue after move sequence
 */

#include "position.hpp" 
#include "uci.hpp"
#include <iostream>

int main() {
    std::cout << "=== Debugging Side-to-Move Issue ===\n";
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "Starting position:\n";
    std::cout << "  FEN: " << pos.to_fen() << "\n";
    std::cout << "  Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    
    // Apply the moves one by one and check side-to-move
    std::vector<std::string> moves = {"c2c4", "g8f6", "d2d4", "e7e6", "g1f3", "b7b6", "b1c3", "c8b7", "a2a3", "d7d5"};
    
    int move_count = 0;
    for (const auto& move_str : moves) {
        move_count++;
        std::cout << "\nMove " << move_count << ": " << move_str;
        
        // Parse UCI move
        S_MOVE move = parse_uci_move(move_str, pos);
        if (move.move == 0) {
            std::cout << " - FAILED to parse\n";
            break;
        }
        
        // Check what side should be making this move
        Color expected_side = (move_count % 2 == 1) ? Color::White : Color::Black;
        std::cout << " (expected " << (expected_side == Color::White ? "White" : "Black") << ")";
        
        if (pos.side_to_move != expected_side) {
            std::cout << " - ERROR: Position says " << (pos.side_to_move == Color::White ? "White" : "Black") << " to move!";
        }
        
        // Make the move
        if (pos.MakeMove(move) != 1) {
            std::cout << " - ILLEGAL move\n";
            break;
        }
        
        std::cout << " - SUCCESS\n";
        std::cout << "  After move " << move_count << ":\n";
        std::cout << "    FEN: " << pos.to_fen() << "\n";
        std::cout << "    Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    }
    
    std::cout << "\n=== Final Analysis ===\n";
    std::cout << "Move count: " << move_count << "\n";
    std::cout << "Expected side to move: " << (move_count % 2 == 0 ? "White" : "Black") << "\n";
    std::cout << "Actual side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    
    if ((move_count % 2 == 0 && pos.side_to_move == Color::White) ||
        (move_count % 2 == 1 && pos.side_to_move == Color::Black)) {
        std::cout << "Side-to-move is CORRECT\n";
    } else {
        std::cout << "Side-to-move is WRONG - This is the bug!\n";
    }
    
    return 0;
}