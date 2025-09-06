/**
 * Evaluation Symmetry Test with Starting Position
 * 
 * Tests the evaluation function symmetry using the starting position,
 * which should be perfectly symmetric.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/evaluation.hpp"
#include <iostream>
#include <iomanip>

void test_starting_position_symmetry() {
    using namespace Huginn;
    
    std::cout << "Evaluation Symmetry Test - Starting Position\n";
    std::cout << "============================================\n\n";
    
    // Starting position - perfectly symmetric
    std::string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    Position original_pos;
    original_pos.set_from_fen(starting_fen);
    
    std::cout << "Original Starting Position:\n";
    std::cout << "FEN: " << starting_fen << "\n\n";
    
    // Use mirrorBoard to create the mirrored version
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    std::string mirrored_fen = mirrored_pos.to_fen();
    
    std::cout << "Mirrored Starting Position:\n";
    std::cout << "FEN: " << mirrored_fen << "\n\n";
    
    // Evaluate both positions
    int original_eval = engine.evaluate(original_pos);
    int mirrored_eval = engine.evaluate(mirrored_pos);
    
    std::cout << "Evaluation Results:\n";
    std::cout << "===================\n";
    std::cout << "Original evaluation:  " << std::setw(4) << original_eval << "\n";
    std::cout << "Mirrored evaluation:  " << std::setw(4) << mirrored_eval << "\n";
    std::cout << "Difference:           " << std::setw(4) << (original_eval - mirrored_eval) << "\n\n";
    
    if (original_eval == -mirrored_eval) {
        std::cout << "✓ PERFECT SYMMETRY! Evaluation function is symmetric.\n";
    } else {
        std::cout << "❌ ASYMMETRY in evaluation function!\n";
        std::cout << "Expected: " << original_eval << " vs " << (-original_eval) << "\n";
        std::cout << "Actual:   " << original_eval << " vs " << mirrored_eval << "\n";
    }
}

void test_simple_symmetric_position() {
    using namespace Huginn;
    
    std::cout << "\n\nSimple Symmetric Position Test\n";
    std::cout << "==============================\n\n";
    
    // Simple position: just kings and one pawn each in symmetric positions
    std::string symmetric_fen = "4k3/4p3/8/8/8/8/4P3/4K3 w - - 0 1";
    
    Position original_pos;
    original_pos.set_from_fen(symmetric_fen);
    
    std::cout << "Original Symmetric Position:\n";
    std::cout << "FEN: " << symmetric_fen << "\n\n";
    
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    std::string mirrored_fen = mirrored_pos.to_fen();
    
    std::cout << "Mirrored Position:\n";
    std::cout << "FEN: " << mirrored_fen << "\n\n";
    
    int original_eval = engine.evaluate(original_pos);
    int mirrored_eval = engine.evaluate(mirrored_pos);
    
    std::cout << "Evaluation Results:\n";
    std::cout << "===================\n";
    std::cout << "Original evaluation:  " << std::setw(4) << original_eval << "\n";
    std::cout << "Mirrored evaluation:  " << std::setw(4) << mirrored_eval << "\n";
    std::cout << "Difference:           " << std::setw(4) << (original_eval - mirrored_eval) << "\n\n";
    
    if (original_eval == -mirrored_eval) {
        std::cout << "✓ PERFECT SYMMETRY!\n";
    } else {
        std::cout << "❌ ASYMMETRY detected!\n";
    }
}

int main() {
    Huginn::init();
    test_starting_position_symmetry();
    test_simple_symmetric_position();
    return 0;
}
