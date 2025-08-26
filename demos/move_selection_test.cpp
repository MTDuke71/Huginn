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
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    
    std::cout << "Evaluating key moves:\n";
    
    for (int i = 0; i < legal_moves.count; ++i) {
        S_MOVE move = legal_moves.moves[i];
        
        // Apply the move
        Position temp_pos = pos;
        temp_pos.make_move_with_undo(move);
        
        int eval = -Evaluation::evaluate_position(temp_pos); // Negative because it's opponent's eval
        
        int from = move.get_from();
        int to = move.get_to();
        
        // Calculate square numbers using sq(File, Rank) 
        int f7_sq = sq(File::F, Rank::R7);  // f7 = 47
        int f6_sq = sq(File::F, Rank::R6);  // f6 = 46
        int e8_sq = sq(File::E, Rank::R8);  // e8 = 95
        int e7_sq = sq(File::E, Rank::R7);  // e7 = 85
        int g8_sq = sq(File::G, Rank::R8);  // g8 = 97
        int b8_sq = sq(File::B, Rank::R8);  // b8 = 92
        int c6_sq = sq(File::C, Rank::R6);  // c6 = 63
        int d7_sq = sq(File::D, Rank::R7);  // d7 = 84
        int d6_sq = sq(File::D, Rank::R6);  // d6 = 74
        int e6_sq = sq(File::E, Rank::R6);  // e6 = 75
        int c7_sq = sq(File::C, Rank::R7);  // c7 = 83
        int c5_sq = sq(File::C, Rank::R5);  // c5 = 73
        
        // Check if this is one of the problematic moves
        if ((from == f7_sq && to == f6_sq) ||   // f7-f6 (terrible f6 pawn move)
            (from == e8_sq && to == e7_sq) ||   // e8-e7 (king walk)
            (from == e8_sq && to == f7_sq)) {   // e8-f7 (king walk)
            std::cout << "BAD Move: from=" << from << " to=" << to << " = " << eval << "cp\n";
        }
        
        // Also show some good moves for comparison
        if ((from == g8_sq && to == f6_sq) ||   // g8-f6 (Nf6 - good development)
            (from == b8_sq && to == c6_sq) ||   // b8-c6 (Nc6 - good development)
            (from == d7_sq && to == d6_sq) ||   // d7-d6 (solid setup)
            (from == e7_sq && to == e6_sq) ||   // e7-e6 (solid French/Caro setup)
            (from == c7_sq && to == c5_sq)) {   // c7-c5 (Sicilian setup)
            std::cout << "GOOD Move: from=" << from << " to=" << to << " = " << eval << "cp\n";
        }
    }
    
    return 0;
}
