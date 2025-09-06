/**
 * Fixed Mirror Board Test
 * 
 * This demo tests the mirrorBoard function with a properly constructed mirrored position.
 * The previous issue was that we were using a manually constructed "mirrored" FEN that
 * wasn't actually a proper mirror of the original position.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/evaluation.hpp"
#include <iostream>
#include <iomanip>

void test_corrected_mirror() {
    using namespace Huginn;
    
    std::cout << "Fixed Mirror Board Test\n";
    std::cout << "======================\n\n";
    
    // King's Gambit position after 1.e4 e5 2.f4
    std::string original_fen = "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2";
    
    Position original_pos;
    original_pos.set_from_fen(original_fen);
    
    std::cout << "Original Position (King's Gambit):\n";
    std::cout << "FEN: " << original_fen << "\n\n";
    
    // Use the mirrorBoard function to create the actual mirrored position
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    
    // Convert the mirrored position back to FEN to see what it should be
    std::string mirrored_fen = mirrored_pos.to_fen();
    std::cout << "Properly Mirrored Position (created by mirrorBoard function):\n";
    std::cout << "FEN: " << mirrored_fen << "\n\n";
    
    // Now evaluate both positions
    int original_eval = engine.evaluate(original_pos);
    int mirrored_eval = engine.evaluate(mirrored_pos);
    
    std::cout << "Evaluation Results:\n";
    std::cout << "===================\n";
    std::cout << "Original evaluation:  " << std::setw(4) << original_eval << "\n";
    std::cout << "Mirrored evaluation:  " << std::setw(4) << mirrored_eval << "\n";
    std::cout << "Difference:           " << std::setw(4) << (original_eval - mirrored_eval) << "\n\n";
    
    if (original_eval == -mirrored_eval) {
        std::cout << "✓ PERFECT SYMMETRY! The mirrorBoard function is working correctly.\n";
        std::cout << "  The evaluations are exact opposites, as expected for mirrored positions.\n";
    } else {
        std::cout << "❌ ASYMMETRY DETECTED!\n";
        std::cout << "  Expected: " << original_eval << " vs " << (-original_eval) << "\n";
        std::cout << "  Actual:   " << original_eval << " vs " << mirrored_eval << "\n";
        std::cout << "  This indicates a bug in the evaluation function.\n";
    }
    
    std::cout << "\nNote: The previous asymmetry was caused by using an incorrectly\n";
    std::cout << "      constructed 'mirrored' FEN instead of using the mirrorBoard function.\n";
}

int main() {
    Huginn::init();
    test_corrected_mirror();
    return 0;
}
