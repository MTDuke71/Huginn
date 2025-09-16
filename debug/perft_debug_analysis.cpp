#include "../src/position.hpp"
#include "../src/init.hpp"
#include "../src/movegen_enhanced.hpp"
#include <iostream>

// Copy of the perft function from perft_huginn2.cpp to debug
uint64_t debug_perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    uint64_t nodes = 0;
    S_MOVELIST move_list;
    
    // Generate all legal moves using bitboard implementation
    generate_legal_moves_enhanced(pos, move_list);
    
    std::cout << "  Depth " << depth << ": Generated " << move_list.count << " moves" << std::endl;
    
    for (int i = 0; i < move_list.count; ++i) {
        const S_MOVE& move = move_list.moves[i];
        
        // Convert move to string for debugging
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
        
        // Make the move
        int make_result = pos.MakeMove(move);
        if (make_result == 1) {
            if (depth == 1) {
                std::cout << "    Move " << i+1 << ": " << move_str << " - LEGAL (will count as 1 node)" << std::endl;
            }
            nodes += debug_perft(pos, depth - 1);
            pos.TakeMove(); // Unmake the move
        } else {
            std::cout << "    Move " << i+1 << ": " << move_str << " - REJECTED by MakeMove" << std::endl;
        }
    }
    
    return nodes;
}

int main() {
    std::cout << "Perft Debug Analysis" << std::endl;
    std::cout << "====================" << std::endl << std::endl;
    
    Huginn::init();
    Position pos;
    pos.set_startpos();
    
    std::cout << "Starting position perft debug:" << std::endl;
    
    // Test depth 1 with detailed logging
    uint64_t result = debug_perft(pos, 1);
    
    std::cout << std::endl;
    std::cout << "Perft depth 1 result: " << result << std::endl;
    std::cout << "Expected: 20" << std::endl;
    
    return 0;
}