/**
 * Asymmetry Investigation Demo
 * 
 * This demo investigates the evaluation asymmetry detected by mirrorBoard.
 * It tests the evaluation function with and without the side-to-move flip
 * to identify the source of the asymmetry.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <iomanip>

void investigate_asymmetry() {
    using namespace Huginn;
    
    std::cout << "Asymmetry Investigation\n";
    std::cout << "=======================\n\n";
    
    // King's Gambit position that shows asymmetry
    std::string original_fen = "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2";
    std::string mirrored_fen = "rnbqkbnr/pppp2pp/8/4pp2/4P3/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 2";
    
    Position original_pos, mirrored_pos;
    original_pos.set_from_fen(original_fen);
    mirrored_pos.set_from_fen(mirrored_fen);
    
    MinimalEngine engine;
    
    std::cout << "Original Position (Black to move):\n";
    std::cout << "FEN: " << original_fen << "\n";
    int original_eval = engine.evaluate(original_pos);
    std::cout << "Evaluation: " << original_eval << "\n\n";
    
    std::cout << "Mirrored Position (White to move):\n";
    std::cout << "FEN: " << mirrored_fen << "\n";
    int mirrored_eval = engine.evaluate(mirrored_pos);
    std::cout << "Evaluation: " << mirrored_eval << "\n\n";
    
    std::cout << "Difference: " << (original_eval - mirrored_eval) << "\n\n";
    
    // Now test with swapped sides to move
    std::cout << "Testing with swapped side to move:\n";
    std::cout << "==================================\n\n";
    
    // Create copies with flipped side to move
    Position original_flipped = original_pos;
    Position mirrored_flipped = mirrored_pos;
    
    original_flipped.side_to_move = (original_pos.side_to_move == Color::White) ? Color::Black : Color::White;
    mirrored_flipped.side_to_move = (mirrored_pos.side_to_move == Color::White) ? Color::Black : Color::White;
    
    std::cout << "Original Position (White to move):\n";
    int original_flipped_eval = engine.evaluate(original_flipped);
    std::cout << "Evaluation: " << original_flipped_eval << "\n\n";
    
    std::cout << "Mirrored Position (Black to move):\n";
    int mirrored_flipped_eval = engine.evaluate(mirrored_flipped);
    std::cout << "Evaluation: " << mirrored_flipped_eval << "\n\n";
    
    std::cout << "Analysis:\n";
    std::cout << "=========\n";
    std::cout << "Original (Black to move): " << std::setw(6) << original_eval << "\n";
    std::cout << "Original (White to move): " << std::setw(6) << original_flipped_eval << "\n";
    std::cout << "Mirrored (White to move): " << std::setw(6) << mirrored_eval << "\n";
    std::cout << "Mirrored (Black to move): " << std::setw(6) << mirrored_flipped_eval << "\n\n";
    
    std::cout << "Side-to-move effect on original: " << (original_eval - original_flipped_eval) << "\n";
    std::cout << "Side-to-move effect on mirrored: " << (mirrored_eval - mirrored_flipped_eval) << "\n\n";
    
    // The issue is likely that the evaluation includes a side-to-move flip
    // Let's see if removing that makes them symmetric
    std::cout << "Hypothesis: The asymmetry comes from the side-to-move flip in evaluation\n";
    std::cout << "If we negate the side-to-move effect:\n";
    
    int original_normalized = (original_pos.side_to_move == Color::White) ? original_eval : -original_eval;
    int mirrored_normalized = (mirrored_pos.side_to_move == Color::White) ? mirrored_eval : -mirrored_eval;
    
    std::cout << "Original normalized (from White's perspective): " << original_normalized << "\n";
    std::cout << "Mirrored normalized (from White's perspective): " << mirrored_normalized << "\n";
    std::cout << "Difference after normalization: " << (original_normalized - mirrored_normalized) << "\n\n";
    
    if (std::abs(original_normalized - mirrored_normalized) < 10) {
        std::cout << "✓ CONFIRMED: The asymmetry is caused by the side-to-move flip!\n";
        std::cout << "The evaluation function negates the score based on side to move,\n";
        std::cout << "but mirrored positions have opposite sides to move.\n";
    } else {
        std::cout << "✗ The asymmetry persists - there may be other issues.\n";
    }
}

int main() {
    Huginn::init();
    investigate_asymmetry();
    return 0;
}
