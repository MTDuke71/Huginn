#include "evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "search.hpp"
#include <iostream>

int main() {
    std::cout << "Analyzing White Opening Moves\n";
    std::cout << "============================\n\n";
    
    // Test White's opening moves from starting position
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Starting position - White to move\n";
    std::cout << "Evaluating common opening moves:\n\n";
    
    // Generate and evaluate opening moves
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    
    // Square calculations for common opening moves
    int e2_sq = sq(File::E, Rank::R2);  // e2 = 34
    int e4_sq = sq(File::E, Rank::R4);  // e4 = 54
    int d2_sq = sq(File::D, Rank::R2);  // d2 = 33
    int d4_sq = sq(File::D, Rank::R4);  // d4 = 53
    int c2_sq = sq(File::C, Rank::R2);  // c2 = 32
    int c3_sq = sq(File::C, Rank::R3);  // c3 = 43
    int c4_sq = sq(File::C, Rank::R4);  // c4 = 52
    int d3_sq = sq(File::D, Rank::R3);  // d3 = 43
    int g1_sq = sq(File::G, Rank::R1);  // g1 = 16
    int f3_sq = sq(File::F, Rank::R3);  // f3 = 45
    int b1_sq = sq(File::B, Rank::R1);  // b1 = 11
    int c3_knight_sq = sq(File::C, Rank::R3);  // c3 = 43
    int f1_sq = sq(File::F, Rank::R1);  // f1 = 15
    int c4_bishop_sq = sq(File::C, Rank::R4);  // c4 = 52
    
    for (int i = 0; i < legal_moves.count; ++i) {
        S_MOVE move = legal_moves.moves[i];
        
        // Apply the move
        Position temp_pos = pos;
        temp_pos.make_move_with_undo(move);
        
        int eval = Evaluation::evaluate_position(temp_pos);
        
        int from = move.get_from();
        int to = move.get_to();
        
        // Check for common opening moves
        if ((from == e2_sq && to == e4_sq)) {   // e2-e4 (King's pawn)
            std::cout << "CLASSIC: e2-e4 = " << eval << "cp (King's Pawn Opening)\n";
        }
        else if ((from == d2_sq && to == d4_sq)) {   // d2-d4 (Queen's pawn)
            std::cout << "CLASSIC: d2-d4 = " << eval << "cp (Queen's Pawn Opening)\n";
        }
        else if ((from == g1_sq && to == f3_sq)) {   // g1-f3 (King's knight)
            std::cout << "GOOD: Nf3 = " << eval << "cp (King's Knight Development)\n";
        }
        else if ((from == b1_sq && to == c3_knight_sq)) {   // b1-c3 (Queen's knight)
            std::cout << "GOOD: Nc3 = " << eval << "cp (Queen's Knight Development)\n";
        }
        else if ((from == f1_sq && to == c4_bishop_sq)) {   // f1-c4 (Bishop development)
            std::cout << "GOOD: Bc4 = " << eval << "cp (Bishop Development)\n";
        }
        else if ((from == c2_sq && to == c3_sq)) {   // c2-c3 (PASSIVE!)
            std::cout << "PASSIVE: c2-c3 = " << eval << "cp (Slow, non-developing)\n";
        }
        else if ((from == c2_sq && to == c4_sq)) {   // c2-c4 (English Opening)
            std::cout << "GOOD: c2-c4 = " << eval << "cp (English Opening)\n";
        }
        else if ((from == d2_sq && to == d3_sq)) {   // d2-d3 (PASSIVE!)
            std::cout << "PASSIVE: d2-d3 = " << eval << "cp (Slow, blocks bishop)\n";
        }
    }
    
    std::cout << "\nRecommendation: Engine should prefer developing moves (Nf3, e4, d4, Nc3) over slow pawn pushes (c3, d3)\n";
    
    return 0;
}
