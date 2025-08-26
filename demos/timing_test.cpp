#include "evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "search.hpp"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "Timing Single Game Analysis\n";
    std::cout << "===========================\n\n";
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    int move_count = 0;
    std::cout << "Playing a single game with timing...\n";
    
    for (int turn = 1; turn <= 100; ++turn) {
        auto move_start = std::chrono::high_resolution_clock::now();
        
        S_MOVELIST legal_moves;
        generate_legal_moves_enhanced(pos, legal_moves);
        
        if (legal_moves.count == 0) {
            std::cout << "Game ended - no legal moves\n";
            break;
        }
        
        // Find best move (same logic as self-play tester)
        S_MOVE best_move = legal_moves.moves[0];
        int best_eval = -999999;
        
        for (int i = 0; i < legal_moves.count; ++i) {
            Position temp_pos = pos;
            temp_pos.make_move_with_undo(legal_moves.moves[i]);
            int eval = -Evaluation::evaluate_position(temp_pos);
            
            if (eval > best_eval) {
                best_eval = eval;
                best_move = legal_moves.moves[i];
            }
        }
        
        pos.make_move_with_undo(best_move);
        move_count++;
        
        auto move_end = std::chrono::high_resolution_clock::now();
        auto move_duration = std::chrono::duration_cast<std::chrono::microseconds>(move_end - move_start);
        
        if (turn <= 5 || turn % 20 == 0) {
            std::cout << "Move " << turn << ": " << move_duration.count() << " microseconds ("
                      << legal_moves.count << " moves evaluated)\n";
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "\n=== TIMING RESULTS ===\n";
    std::cout << "Total moves: " << move_count << "\n";
    std::cout << "Total time: " << total_duration.count() << " milliseconds\n";
    std::cout << "Average time per move: " << (double)total_duration.count() / move_count << " ms\n";
    std::cout << "Moves per second: " << (1000.0 * move_count) / total_duration.count() << "\n";
    
    return 0;
}
