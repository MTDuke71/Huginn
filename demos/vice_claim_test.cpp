/**
 * VICE Tutorial Claim Validation: e2-e4 c7-c6 Symmetry Test
 * 
 * Tests the VICE video claim that the position after 1.e4 c6 
 * should result in symmetric scores when mirrored.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/evaluation.hpp"
#include <iostream>
#include <iomanip>

void test_e4_c6_symmetry() {
    using namespace Huginn;
    
    std::cout << "VICE Tutorial Claim Test: 1.e4 c6 Symmetry\n";
    std::cout << "==========================================\n\n";
    
    // Position after 1.e4 c6
    std::string e4_c6_fen = "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";
    
    Position original_pos;
    original_pos.set_from_fen(e4_c6_fen);
    
    std::cout << "Original Position (1.e4 c6):\n";
    std::cout << "============================\n";
    std::cout << "FEN: " << e4_c6_fen << "\n";
    std::cout << "White: e2 pawn moved to e4\n";
    std::cout << "Black: c7 pawn moved to c6\n\n";
    
    // Create mirrored position using mirrorBoard function
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    std::string mirrored_fen = mirrored_pos.to_fen();
    
    std::cout << "Mirrored Position (created by mirrorBoard):\n";
    std::cout << "===========================================\n";
    std::cout << "FEN: " << mirrored_fen << "\n\n";
    
    // Evaluate both positions
    int original_eval = engine.evaluate(original_pos);
    int mirrored_eval = engine.evaluate(mirrored_pos);
    
    std::cout << "Evaluation Results:\n";
    std::cout << "===================\n";
    std::cout << "Original evaluation:  " << std::setw(4) << original_eval << "\n";
    std::cout << "Mirrored evaluation:  " << std::setw(4) << mirrored_eval << "\n";
    std::cout << "Difference:           " << std::setw(4) << (original_eval - mirrored_eval) << "\n";
    std::cout << "Sum (should be 0):    " << std::setw(4) << (original_eval + mirrored_eval) << "\n\n";
    
    // Test VICE claim
    if (original_eval == -mirrored_eval) {
        std::cout << "✓ VICE CLAIM CONFIRMED!\n";
        std::cout << "  The positions show perfect symmetry as claimed in the tutorial.\n";
        std::cout << "  Evaluations are exact opposites: " << original_eval << " vs " << mirrored_eval << "\n";
    } else {
        std::cout << "❌ VICE CLAIM NOT CONFIRMED!\n";
        std::cout << "  Expected: " << original_eval << " vs " << (-original_eval) << "\n";
        std::cout << "  Actual:   " << original_eval << " vs " << mirrored_eval << "\n";
        std::cout << "  Asymmetry: " << (original_eval + mirrored_eval) << " points\n";
    }
    
    // Let's also test what the manually constructed symmetric position would be
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "Manual Symmetric Position Test\n";
    std::cout << "==============================\n\n";
    
    // What should the symmetric position actually be?
    // If White played e4, then in the mirrored game, Black should play e5
    // If Black played c6, then in the mirrored game, White should play c3
    std::string expected_symmetric_fen = "rnbqkbnr/pppp1ppp/8/4p3/8/2P5/PP1PPPPP/RNBQKBNR b KQkq - 0 2";
    
    Position expected_pos;
    expected_pos.set_from_fen(expected_symmetric_fen);
    
    std::cout << "Expected Truly Symmetric Position (1.c3 e5):\n";
    std::cout << "============================================\n";
    std::cout << "FEN: " << expected_symmetric_fen << "\n";
    std::cout << "White: c2 pawn moved to c3 (mirrors Black's c7-c6)\n";
    std::cout << "Black: e7 pawn moved to e5 (mirrors White's e2-e4)\n\n";
    
    Position expected_mirrored = engine.mirrorBoard(expected_pos);
    int expected_eval = engine.evaluate(expected_pos);
    int expected_mirrored_eval = engine.evaluate(expected_mirrored);
    
    std::cout << "Expected Position Evaluation: " << expected_eval << "\n";
    std::cout << "Expected Mirrored Evaluation: " << expected_mirrored_eval << "\n";
    std::cout << "Expected Symmetry Test: " << (expected_eval == -expected_mirrored_eval ? "✓ PASS" : "❌ FAIL") << "\n";
}

int main() {
    Huginn::init();
    test_e4_c6_symmetry();
    return 0;
}
