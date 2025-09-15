/**
 * @file debug_makemove_issue.cpp
 * @brief Debug MakeMove function step by step to find where position breaks
 */

#include "position.hpp" 
#include "uci.hpp"
#include <iostream>

void print_position_state(const Position& pos, const std::string& label) {
    std::cout << "\n=== " << label << " ===\n";
    std::cout << "FEN: " << pos.to_fen() << "\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    std::cout << "Move number: " << pos.full_move_counter << "\n";
    std::cout << "Half-move clock: " << pos.half_move_counter << "\n";
    std::cout << "En passant square: " << pos.ep_square << "\n";
    std::cout << "Castling rights: " << pos.castling_rights << "\n";
}

int main() {
    std::cout << "=== Step-by-Step MakeMove Debug ===\n";
    
    Position pos;
    pos.set_startpos();
    print_position_state(pos, "Starting Position");
    
    // Test the first few moves to see where it breaks
    std::vector<std::string> moves = {"c2c4", "g8f6", "d2d4", "e7e6", "g1f3"};
    
    for (size_t i = 0; i < moves.size(); ++i) {
        const std::string& move_str = moves[i];
        
        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << "APPLYING MOVE " << (i+1) << ": " << move_str << "\n";
        std::cout << std::string(50, '=') << "\n";
        
        // Parse the move
        S_MOVE move = parse_uci_move(move_str, pos);
        if (move.move == 0) {
            std::cout << "ERROR: Failed to parse move " << move_str << "\n";
            break;
        }
        
        std::cout << "Parsed move successfully: " << move_to_uci(move) << "\n";
        std::cout << "Move bits: " << std::hex << move.move << std::dec << "\n";
        
        // Check side to move BEFORE applying move
        Color expected_side = (i % 2 == 0) ? Color::White : Color::Black;
        std::cout << "Expected side to move: " << (expected_side == Color::White ? "White" : "Black") << "\n";
        std::cout << "Actual side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
        
        if (pos.side_to_move != expected_side) {
            std::cout << "ERROR: Side to move is wrong BEFORE applying move!\n";
            break;
        }
        
        print_position_state(pos, "BEFORE MakeMove");
        
        // Apply the move
        int make_result = pos.MakeMove(move);
        std::cout << "\nMakeMove returned: " << make_result << "\n";
        
        if (make_result != 1) {
            std::cout << "ERROR: MakeMove failed with code " << make_result << "\n";
            break;
        }
        
        print_position_state(pos, "AFTER MakeMove");
        
        // Verify the side switched correctly
        Color new_expected_side = (expected_side == Color::White) ? Color::Black : Color::White;
        if (pos.side_to_move != new_expected_side) {
            std::cout << "ERROR: Side to move did not flip correctly!\n";
            std::cout << "Expected after move: " << (new_expected_side == Color::White ? "White" : "Black") << "\n";
            std::cout << "Actual after move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
            break;
        }
        
        std::cout << "✓ Move applied successfully\n";
    }
    
    return 0;
}