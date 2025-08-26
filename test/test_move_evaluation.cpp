#include "src/position.hpp"
#include "src/evaluation.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/search.hpp"
#include "src/attack_detection.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>

struct MoveEval {
    S_MOVE move;
    int eval;
    bool is_mate;
    bool is_check;
    int legal_responses;
};

int main() {
    std::cout << "=== MOVE EVALUATION COMPARISON ===" << std::endl;
    
    // Test the simple mate position
    Position pos;
    pos.set_from_fen("k7/8/1K6/8/8/8/8/7Q w - - 0 1");
    std::cout << "Position: " << pos.to_fen() << std::endl;
    
    // Generate all legal moves
    S_MOVELIST moves;
    generate_legal_moves_enhanced(pos, moves);
    
    std::vector<MoveEval> move_evals;
    
    std::cout << "\nEvaluating all " << moves.count << " moves:" << std::endl;
    
    for (int i = 0; i < moves.count; ++i) {
        const S_MOVE& move = moves.moves[i];
        std::string move_str = Search::move_to_uci(move);
        
        // Make the move
        pos.make_move_with_undo(move);
        
        // Evaluate the position from the perspective of the side that MADE the move
        // The position now has the opponent to move, so we need to negate the evaluation
        int eval = -Evaluation::evaluate_position(pos);
        
        // Check if opponent is in check
        int opp_king_sq = pos.king_sq[int(pos.side_to_move)];
        bool in_check = (opp_king_sq >= 0) && SqAttacked(opp_king_sq, pos, !pos.side_to_move);
        
        // Count opponent's legal moves
        S_MOVELIST opp_moves;
        generate_legal_moves_enhanced(pos, opp_moves);
        
        // Check if it's mate
        bool is_mate = (opp_moves.count == 0 && in_check);
        
        // Undo the move
        pos.undo_move();
        
        MoveEval me;
        me.move = move;
        me.eval = eval;
        me.is_mate = is_mate;
        me.is_check = in_check;
        me.legal_responses = opp_moves.count;
        
        move_evals.push_back(me);
        
        std::cout << std::setw(6) << move_str 
                  << " eval=" << std::setw(6) << eval
                  << " check=" << (in_check ? "Y" : "N")
                  << " mate=" << (is_mate ? "Y" : "N")
                  << " responses=" << opp_moves.count << std::endl;
    }
    
    // Sort by evaluation (best for white = highest score)
    std::sort(move_evals.begin(), move_evals.end(), 
              [](const MoveEval& a, const MoveEval& b) {
                  return a.eval > b.eval;
              });
    
    std::cout << "\nMoves sorted by evaluation (best first):" << std::endl;
    for (size_t i = 0; i < std::min(size_t(10), move_evals.size()); ++i) {
        const auto& me = move_evals[i];
        std::string move_str = Search::move_to_uci(me.move);
        std::cout << (i+1) << ". " << std::setw(6) << move_str 
                  << " eval=" << std::setw(6) << me.eval
                  << " mate=" << (me.is_mate ? "Y" : "N") << std::endl;
    }
    
    // Find mating moves
    std::cout << "\nMating moves found:" << std::endl;
    for (const auto& me : move_evals) {
        if (me.is_mate) {
            std::string move_str = Search::move_to_uci(me.move);
            std::cout << "  " << move_str << " eval=" << me.eval << std::endl;
        }
    }
    
    return 0;
}
