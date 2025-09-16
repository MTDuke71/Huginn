#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/movegen_enhanced.hpp"
#include <iostream>

int main() {
    std::cout << "MakeMove Legality Filter Analysis" << std::endl;
    std::cout << "=================================" << std::endl << std::endl;
    
    Huginn::init();
    Position pos;
    pos.set_startpos();
    
    // Generate pseudo-legal moves
    S_MOVELIST pseudo_moves;
    generate_all_moves(pos, pseudo_moves);
    
    std::cout << "Pseudo-legal moves generated: " << pseudo_moves.count << std::endl;
    
    // Test each move with MakeMove
    int legal_count = 0;
    int illegal_count = 0;
    
    std::cout << "\nTesting each move with MakeMove:" << std::endl;
    
    for (int i = 0; i < pseudo_moves.count; i++) {
        const S_MOVE& move = pseudo_moves.moves[i];
        Position temp_pos = pos;  // Work on copy
        
        // Convert move to string
        int from = move.get_from();
        int to = move.get_to();
        int from_file = MAILBOX_MAPS.to64[from] % 8;
        int from_rank = MAILBOX_MAPS.to64[from] / 8;
        int to_file = MAILBOX_MAPS.to64[to] % 8;
        int to_rank = MAILBOX_MAPS.to64[to] / 8;
        
        std::string move_str = "";
        move_str += ('a' + from_file);
        move_str += ('1' + from_rank);
        move_str += ('a' + to_file);
        move_str += ('1' + to_rank);
        
        // Test legality
        int result = temp_pos.MakeMove(move);
        if (result == 1) {
            std::cout << move_str << " - LEGAL" << std::endl;
            legal_count++;
            temp_pos.TakeMove();
        } else {
            std::cout << move_str << " - ILLEGAL (rejected by MakeMove)" << std::endl;
            illegal_count++;
        }
    }
    
    std::cout << std::endl;
    std::cout << "Summary:" << std::endl;
    std::cout << "Total pseudo-legal moves: " << pseudo_moves.count << std::endl;
    std::cout << "Legal moves: " << legal_count << std::endl;
    std::cout << "Illegal moves: " << illegal_count << std::endl;
    std::cout << "Expected legal moves: 20" << std::endl;
    
    return 0;
}