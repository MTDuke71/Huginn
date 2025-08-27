#include "../src/evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>

int main() {
    std::cout << "Testing Engine's Move Selection\n";
    std::cout << "==============================\n\n";
    
    // First test: White opening moves from starting position
    Position start_pos;
    start_pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Get baseline evaluation from White's perspective  
    int baseline_eval = Huginn::HybridEvaluator::evaluate(start_pos);
    std::cout << "Starting position evaluation: " << baseline_eval << "cp (White's perspective)\n\n";
    
    std::cout << "WHITE OPENING MOVES (Starting Position)\n";
    std::cout << "=======================================\n";
    
    S_MOVELIST start_moves;
    generate_legal_moves_enhanced(start_pos, start_moves);
    
    for (int i = 0; i < start_moves.count; ++i) {
        S_MOVE move = start_moves.moves[i];
        Position temp_pos = start_pos;
        temp_pos.make_move_with_undo(move);
        
        int eval = Huginn::HybridEvaluator::evaluate(temp_pos);
        // Convert all evaluations to White's perspective for comparison
        int eval_from_white_perspective = (temp_pos.side_to_move == Color::White) ? eval : -eval;
        int baseline_from_white_perspective = (start_pos.side_to_move == Color::White) ? baseline_eval : -baseline_eval;
        int eval_change = eval_from_white_perspective - baseline_from_white_perspective;  // Positive means good for white
        int from = move.get_from();
        int to = move.get_to();
        
        // Check for key opening moves
        if ((from == 34 && to == 54)) {   // e2-e4
            std::cout << "CLASSIC: e2-e4 = " << eval_change << "cp change (King's Pawn) [pos=" << eval << "cp]\n";
            
            // Debug breakdown for e2-e4
            int pos_change = Huginn::HybridEvaluator::evaluate(temp_pos) - Huginn::HybridEvaluator::evaluate(start_pos);
            // Individual component evaluation not exposed in unified evaluator
            std::cout << "  -> Positional change: " << pos_change << "cp (unified evaluation)\n";
        }
        else if ((from == 33 && to == 53)) {   // d2-d4
            std::cout << "CLASSIC: d2-d4 = " << eval_change << "cp change (Queen's Pawn) [pos=" << eval << "cp]\n";
            
            // Debug breakdown for d2-d4
            int pos_change = Huginn::HybridEvaluator::evaluate(temp_pos) - Huginn::HybridEvaluator::evaluate(start_pos);
            // Individual component evaluation not exposed in unified evaluator
            std::cout << "  -> Positional change: " << pos_change << "cp (unified evaluation)\n";
        }
        else if ((from == 16 && to == 45)) {   // g1-f3
            std::cout << "GOOD: Nf3 = " << eval_change << "cp change (Knight Development) [pos=" << eval << "cp]\n";
        }
        else if ((from == 11 && to == 43)) {   // b1-c3
            std::cout << "GOOD: Nc3 = " << eval_change << "cp change (Knight Development) [pos=" << eval << "cp]\n";
        }
        else if ((from == 32 && to == 52)) {   // c2-c4
            std::cout << "GOOD: c4 = " << eval_change << "cp change (English Opening) [pos=" << eval << "cp]\n";
        }
        else if ((from == 32 && to == 43)) {   // c2-c3 (PASSIVE!)
            std::cout << "PASSIVE: c2-c3 = " << eval_change << "cp change (Slow, non-developing) [pos=" << eval << "cp]\n";
        }
        else if ((from == 33 && to == 44)) {   // d2-d3 (PASSIVE!)
            std::cout << "PASSIVE: d2-d3 = " << eval_change << "cp change (Blocks bishop) [pos=" << eval << "cp]\n";
        }
        else if ((from == 34 && to == 45)) {   // e2-e3
            std::cout << "PASSIVE: e2-e3 = " << eval_change << "cp change (Slow development) [pos=" << eval << "cp]\n";
        }
    }
    
    std::cout << "\n";
    
    // Original test: Black responses after 1.e4
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    
    std::cout << "BLACK RESPONSES (After 1.e4)\n";
    std::cout << "============================\n";
    
    // Generate and evaluate a few common moves
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    
    std::cout << "Evaluating key moves:\n";
    
    for (int i = 0; i < legal_moves.count; ++i) {
        S_MOVE move = legal_moves.moves[i];
        
        // Apply the move
        Position temp_pos = pos;
        temp_pos.make_move_with_undo(move);
        
        int eval = -Huginn::HybridEvaluator::evaluate(temp_pos); // Negative because it's opponent's eval
        
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


