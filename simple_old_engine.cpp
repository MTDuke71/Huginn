#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include <iostream>

void compare_perft_counts(const std::string& description, const std::string& fen) {
    std::cout << "=== " << description << " (OLD ENGINE PERFT) ===\n";
    
    Position pos;
    pos.set_from_fen(fen);
    
    std::cout << "Position: " << fen << "\n";
    
    S_MOVELIST moves;
    generate_legal_moves_enhanced(pos, moves);
    
    std::cout << "Legal move count: " << moves.count << "\n\n";
}

int main() {
    std::cout << "=== OLD ENGINE SIMPLE REFERENCE ===\n\n";
    
    // Get reference move counts for our problem positions
    compare_perft_counts("After 1.h2h4 h7h5", 
                        "rnbqkbnr/ppppppp1/8/7p/7P/8/PPPPPPP1/RNBQKBNR w KQkq h6 0 2");
    
    compare_perft_counts("After 1.h2h4 g7g5", 
                        "rnbqkbnr/pppppp1p/8/6p1/7P/8/PPPPPPP1/RNBQKBNR w KQkq g6 0 2");
    
    compare_perft_counts("After 1.h2h4 e7e5", 
                        "rnbqkbnr/pppp1ppp/8/4p3/7P/8/PPPPPPP1/RNBQKBNR w KQkq e6 0 2");
    
    return 0;
}