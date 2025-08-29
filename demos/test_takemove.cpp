// Simple test to verify VICE TakeMove function works correctly
#include <iostream>
#include "../src/position.hpp"
#include "../src/movegen_enhanced.hpp"

int main() {
    std::cout << "Testing VICE Tutorial Video #42: TakeMove function\n" << std::endl;
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "Starting position:" << std::endl;
    std::cout << pos.to_fen() << std::endl << std::endl;
    
    // Generate a legal move
    S_MOVELIST moves;
    generate_legal_moves_enhanced(pos, moves);
    
    if (moves.count > 0) {
        S_MOVE move = moves.moves[0];  // First legal move
        
        std::cout << "Making move from " << move.get_from() << " to " << move.get_to() << std::endl;
        
        // Store original position
        std::string original_fen = pos.to_fen();
        
        // Make the move
        int result = pos.MakeMove(move);
        std::cout << "MakeMove result: " << result << " (1 = legal, 0 = illegal)" << std::endl;
        
        if (result == 1) {
            std::cout << "Position after move:" << std::endl;
            std::cout << pos.to_fen() << std::endl << std::endl;
            
            // Take the move back
            std::cout << "Taking move back..." << std::endl;
            pos.TakeMove();
            
            std::string restored_fen = pos.to_fen();
            std::cout << "Position after TakeMove:" << std::endl;
            std::cout << restored_fen << std::endl << std::endl;
            
            // Verify positions match
            if (original_fen == restored_fen) {
                std::cout << "✅ SUCCESS: TakeMove correctly restored the position!" << std::endl;
            } else {
                std::cout << "❌ ERROR: Position not correctly restored!" << std::endl;
                std::cout << "Original:  " << original_fen << std::endl;
                std::cout << "Restored:  " << restored_fen << std::endl;
                return 1;
            }
        }
    }
    
    std::cout << "\nTesting with more complex moves..." << std::endl;
    
    // Test with a more complex position (includes castling rights)
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    std::string complex_original = pos.to_fen();
    
    generate_legal_moves_enhanced(pos, moves);
    for (int i = 0; i < std::min(5, moves.count); ++i) {
        S_MOVE test_move = moves.moves[i];
        
        if (pos.MakeMove(test_move) == 1) {
            pos.TakeMove();
            
            if (pos.to_fen() != complex_original) {
                std::cout << "❌ ERROR: Complex position not correctly restored on move " << i << std::endl;
                return 1;
            }
        }
    }
    
    std::cout << "✅ All complex position tests passed!" << std::endl;
    std::cout << "\nVICE TakeMove function is working correctly!" << std::endl;
    
    return 0;
}
