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
    
    std::cout << "=== VICE Debug Test ===" << std::endl;
    std::cout << "Starting position: " << pos.to_fen() << std::endl;
    
    // Test multiple moves
    for (int move_num = 1; move_num <= 5; ++move_num) {
        std::cout << "\n--- Move " << move_num << " ---" << std::endl;
        
        // Generate moves
        S_MOVELIST moves;
        generate_legal_moves_enhanced(pos, moves);
        
        if (moves.count == 0) {
            std::cout << "No legal moves available!" << std::endl;
            break;
        }
        
        // Try first move
        S_MOVE move = moves.moves[0];
        std::cout << "Making move: " << Huginn::SimpleEngine::move_to_uci(move) << std::endl;
        std::cout << "FEN before: " << pos.to_fen() << std::endl;
        
        // Store original position
        std::string orig_fen = pos.to_fen();
        
        // Make the move
        int result = pos.MakeMove(move);
        std::cout << "MakeMove result: " << result << std::endl;
        
        if (result != 1) {
            std::cout << "Move was illegal!" << std::endl;
            break;
        }
        
        std::cout << "FEN after: " << pos.to_fen() << std::endl;
        
        // Take the move back
        std::cout << "Taking move back..." << std::endl;
        pos.TakeMove();
        std::string final_fen = pos.to_fen();
        
        std::cout << "FEN after TakeMove: " << final_fen << std::endl;
        
        // Check if restored correctly
        if (final_fen == orig_fen) {
            std::cout << "✓ Position restored correctly!" << std::endl;
        } else {
            std::cout << "✗ FAILURE: Position not restored!" << std::endl;
            std::cout << "Expected: " << orig_fen << std::endl;
            std::cout << "Actual:   " << final_fen << std::endl;
            return 1;
        }
        
        // Make the move permanently to test next position
        pos.MakeMove(move);
    }
    
    std::cout << "\n=== All tests passed! ===" << std::endl;
    return 0;
}
