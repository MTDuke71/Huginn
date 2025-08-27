#include "../src/evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>

int main() {
    std::cout << "Testing Basic King Walking Detection\n";
    std::cout << "====================================\n\n";
    
    // Test the exact position from the game log
    std::cout << "1. Testing after 1.Nf3 f6:\n";
    Position pos1;
    pos1.set_startpos();
    
    // Make the moves from the game
    S_MOVELIST moves1;
    generate_legal_moves_enhanced(pos1, moves1);
    
    // Find and evaluate f6 move
    for (int i = 0; i < moves1.count; i++) {
        S_MOVE move = moves1.moves[i];
        int from = move.get_from();
        int to = move.get_to();
        
        // Check if this is f7-f6
        if (from == sq(File::F, Rank::R7) && to == sq(File::F, Rank::R6)) {
            Position temp_pos = pos1;
            temp_pos.make_move_with_undo(move);
            int eval = -Huginn::HybridEvaluator::evaluate(temp_pos);
            std::cout << "f6 move evaluation: " << eval << "cp\n";
            temp_pos.undo_move();
            break;
        }
    }
    
    std::cout << "\n2. Testing after 1.Nf3 f6 2.e4 Ke7:\n";
    // Set position after f6
    Position pos2;
    pos2.set_from_fen("rnbqkbnr/ppppp1pp/5p2/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 0 2");
    
    S_MOVELIST moves2;
    generate_legal_moves_enhanced(pos2, moves2);
    
    // Find and evaluate Ke7 move
    for (int i = 0; i < moves2.count; i++) {
        S_MOVE move = moves2.moves[i];
        int from = move.get_from();
        int to = move.get_to();
        
        // Check if this is Ke8-e7
        if (from == sq(File::E, Rank::R8) && to == sq(File::E, Rank::R7)) {
            Position temp_pos = pos2;
            temp_pos.make_move_with_undo(move);
            int eval = -Huginn::HybridEvaluator::evaluate(temp_pos);
            std::cout << "Ke7 move evaluation: " << eval << "cp\n";
            temp_pos.undo_move();
            break;
        }
    }
    
    return 0;
}

