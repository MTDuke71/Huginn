#include <iostream>
#include "position.hpp" 
#include "movegen_enhanced.hpp"
#include "init.hpp"

int main() {
    // Initialize the chess engine
    Huginn::init();
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "=== Position Copy Test ===" << std::endl;
    std::cout << "Starting position: " << pos.to_fen() << std::endl;
    std::cout << "Original position ply: " << pos.ply << std::endl;
    std::cout << "Original move_history size: " << pos.move_history.size() << std::endl;
    
    // Get a legal move
    S_MOVELIST moves;
    generate_legal_moves_enhanced(pos, moves);
    
    if (moves.count == 0) {
        std::cout << "ERROR: No legal moves!" << std::endl;
        return 1;
    }
    
    S_MOVE test_move = moves.moves[0];
    std::cout << "Testing with move: " << std::hex << test_move.move << std::dec << std::endl;
    
    // Test 1: Copy position and then make/unmake move on copy
    std::cout << "\n--- Test 1: Copy Position Before Move ---" << std::endl;
    Position copied_pos = pos;  // This is what score_move() does!
    
    std::cout << "Copied position ply: " << copied_pos.ply << std::endl;
    std::cout << "Copied move_history size: " << copied_pos.move_history.size() << std::endl;
    
    // Make move on copy
    std::cout << "Making move on copied position..." << std::endl;
    int result = copied_pos.MakeMove(test_move);
    if (result == 1) {
        std::cout << "  Move made successfully, new ply: " << copied_pos.ply << std::endl;
        std::cout << "  Move_history size: " << copied_pos.move_history.size() << std::endl;
        
        // Unmake move on copy
        std::cout << "Taking back move on copied position..." << std::endl;
        copied_pos.TakeMove();
        std::cout << "  Move taken back, ply: " << copied_pos.ply << std::endl;
        std::cout << "  ✓ SUCCESS: Position copy works with VICE!" << std::endl;
    } else {
        std::cout << "  ✗ ERROR: Move failed on copied position!" << std::endl;
        return 1;
    }
    
    // Test 2: Make move on original, then copy, then unmake
    std::cout << "\n--- Test 2: Copy Position After Move ---" << std::endl;
    std::cout << "Making move on original position..." << std::endl;
    int orig_result = pos.MakeMove(test_move);
    if (orig_result == 1) {
        std::cout << "  Original position ply: " << pos.ply << std::endl;
        std::cout << "  Original move_history size: " << pos.move_history.size() << std::endl;
        
        // Copy position with move made
        Position copied_after = pos;
        std::cout << "  Copied-after position ply: " << copied_after.ply << std::endl;
        std::cout << "  Copied-after move_history size: " << copied_after.move_history.size() << std::endl;
        
        // Try to unmake on copy
        std::cout << "Taking back move on copied-after position..." << std::endl;
        try {
            copied_after.TakeMove();
            std::cout << "  ✓ SUCCESS: TakeMove worked on copied position!" << std::endl;
            std::cout << "  Copied-after position ply: " << copied_after.ply << std::endl;
        } catch (...) {
            std::cout << "  ✗ ERROR: TakeMove crashed on copied position!" << std::endl;
            return 1;
        }
        
        // Clean up original 
        pos.TakeMove();
        std::cout << "  Original position cleaned up, ply: " << pos.ply << std::endl;
    }
    
    std::cout << "\n=== All Position copy tests passed! ===" << std::endl;
    return 0;
}
