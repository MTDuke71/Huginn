#include <iostream>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "search.hpp"
#include "init.hpp"

int main() {
    // Initialize the chess engine
    Huginn::init();
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "=== Legal Move Generation Test ===" << std::endl;
    std::cout << "Starting position: " << pos.to_fen() << std::endl;
    
    // Generate legal moves
    S_MOVELIST moves;
    generate_legal_moves_enhanced(pos, moves);
    
    std::cout << "Generated " << moves.count << " legal moves:" << std::endl;
    
    // Print first few moves
    for (int i = 0; i < std::min(5, moves.count); ++i) {
        std::cout << "  " << (i+1) << ". " << Huginn::SimpleEngine::move_to_uci(moves.moves[i]) << std::endl;
    }
    
    // Test making and unmaking each move
    int successful_moves = 0;
    for (int i = 0; i < moves.count; ++i) {
        std::string orig_fen = pos.to_fen();
        
        if (pos.MakeMove(moves.moves[i]) == 1) {
            pos.TakeMove();
            std::string final_fen = pos.to_fen();
            
            if (orig_fen == final_fen) {
                successful_moves++;
            } else {
                std::cout << "ERROR: Move " << Huginn::SimpleEngine::move_to_uci(moves.moves[i]) 
                          << " failed to restore position!" << std::endl;
                std::cout << "Original: " << orig_fen << std::endl;
                std::cout << "Final:    " << final_fen << std::endl;
                return 1;
            }
        } else {
            std::cout << "ERROR: Legal move " << Huginn::SimpleEngine::move_to_uci(moves.moves[i]) 
                      << " was rejected by MakeMove!" << std::endl;
            return 1;
        }
    }
    
    std::cout << "SUCCESS: All " << successful_moves << " moves made and unmade correctly!" << std::endl;
    return 0;
}
