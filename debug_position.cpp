#include "position.hpp"
#include "init.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>

int main() {
    Huginn::init();
    
    Position pos;
    pos.set_startpos();
    
    // Apply the moves: b1c3 g8f6 g1f3 b8c6 d2d4 d7d5 d1d3 c8g4 f3e5
    std::vector<std::string> moves = {"b1c3", "g8f6", "g1f3", "b8c6", "d2d4", "d7d5", "d1d3", "c8g4", "f3e5"};
    
    for (const auto& move_str : moves) {
        S_MOVELIST legal_moves;
        generate_legal_moves_enhanced(pos, legal_moves);
        
        S_MOVE move_to_make;
        bool found = false;
        
        // Find the move
        for (int i = 0; i < legal_moves.count; ++i) {
            S_MOVE candidate = legal_moves.moves[i];
            int from = candidate.get_from();
            int to = candidate.get_to();
            
            // Convert to algebraic
            char from_file = 'a' + int(file_of(from));
            char from_rank = '1' + int(rank_of(from));
            char to_file = 'a' + int(file_of(to));
            char to_rank = '1' + int(rank_of(to));
            
            std::string candidate_str = std::string(1, from_file) + from_rank + to_file + to_rank;
            
            if (candidate_str == move_str) {
                move_to_make = candidate;
                found = true;
                break;
            }
        }
        
        if (found) {
            std::cout << "Making move: " << move_str << std::endl;
            pos.make_move_with_undo(move_to_make);
        } else {
            std::cout << "Move not found: " << move_str << std::endl;
            return 1;
        }
    }
    
    // Show king positions
    std::cout << "White king at square: " << pos.king_sq[0] << std::endl;
    std::cout << "Black king at square: " << pos.king_sq[1] << std::endl;
    
    // Generate moves and check for issues
    S_MOVELIST moves_list;
    generate_legal_moves_enhanced(pos, moves_list);
    
    std::cout << "Generated " << moves_list.count << " legal moves" << std::endl;
    
    return 0;
}
