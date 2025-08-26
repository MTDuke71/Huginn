#include "evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "search.hpp"
#include <iostream>

int main() {
    std::cout << "Testing Engine's Move Selection\n";
    std::cout << "==============================\n\n";
    
    // Test if engine will choose f6 as Black in starting position
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    
    std::cout << "Position after 1.e4 - Black to move\n";
    std::cout << "Testing if engine will avoid f6...\n\n";
    
    // Generate and evaluate a few common moves
    MoveList legal_moves;
    generate_legal_moves(pos, legal_moves);
    
    std::cout << "Evaluating key moves:\n";
    
    for (int i = 0; i < legal_moves.size; ++i) {
        Move move = legal_moves.moves[i];
        
        // Apply the move
        Position temp_pos = pos;
        temp_pos.make_move(move);
        
        int eval = -Evaluation::evaluate_position(temp_pos); // Negative because it's opponent's eval
        
        // Check if this is one of the problematic moves
        std::string move_str = move.to_string();
        if (move_str == "f7f6" || move_str == "e8e7" || move_str == "e8f7" || 
            move_str.find("f6") != std::string::npos ||
            move_str.find("e7") != std::string::npos) {
            std::cout << "Move: " << move_str << " = " << eval << "cp\n";
        }
        
        // Also show some good moves for comparison
        if (move_str == "g8f6" || move_str == "b8c6" || move_str == "d7d6" || 
            move_str == "e7e6" || move_str == "c7c5") {
            std::cout << "Move: " << move_str << " = " << eval << "cp\n";
        }
    }
    
    return 0;
}
