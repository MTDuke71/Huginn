#include "../Engine3_src/hybrid_evaluation.hpp"
#include "../Engine3_src/simple_search.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Critical Position Test: After 1.e4\n";
    std::cout << "==================================\n\n";
    
    // Set up position after 1.e4 - this is where the engine was choosing f6
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    
    std::cout << "Position: After 1.e4, Black to move\n";
    std::cout << "Testing what move the engine chooses...\n\n";
    
    // Generate all legal moves and find the best one
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    
    S_MOVE best_move;
    int best_eval = -999999;
    
    for (int i = 0; i < legal_moves.count; ++i) {
        S_MOVE move = legal_moves.moves[i];
        Position temp_pos = pos;
        temp_pos.make_move_with_undo(move);
        
        int eval = -Engine3::HybridEvaluator::evaluate(temp_pos);
        
        if (eval > best_eval) {
            best_eval = eval;
            best_move = move;
        }
    }
    
    // Convert move to notation
    auto square_to_notation = [](int sq) {
        int file = sq % 8;
        int rank = sq / 8;
        char file_char = 'a' + file;
        char rank_char = '1' + rank;
        return std::string(1, file_char) + std::string(1, rank_char);
    };
    
    std::string best_move_notation = square_to_notation(best_move.get_from()) + square_to_notation(best_move.get_to());
    
    std::cout << "Engine's choice: " << best_move_notation << " (eval: " << best_eval << "cp)\n\n";
    
    // Check if it's a problematic move
    if (best_move_notation == "f7f6") {
        std::cout << "❌ CRITICAL FAILURE: Engine still choosing f6!\n";
        std::cout << "The king safety evaluation needs to be strengthened.\n";
    } else if (best_move_notation.find("e8") != std::string::npos) {
        std::cout << "⚠️  WARNING: Engine choosing king move!\n";
        std::cout << "King walking might still be an issue.\n";
    } else {
        std::cout << "✅ SUCCESS: Engine avoiding problematic moves!\n";
        
        // Show what it's choosing instead
        if (best_move_notation == "g8f6") {
            std::cout << "Good choice: Nf6 - solid knight development\n";
        } else if (best_move_notation == "b8c6") {
            std::cout << "Good choice: Nc6 - solid knight development\n";
        } else if (best_move_notation == "e7e6") {
            std::cout << "Good choice: e6 - solid central control\n";
        } else if (best_move_notation == "e7e5") {
            std::cout << "Good choice: e5 - aggressive central control\n";
        } else if (best_move_notation == "c7c5") {
            std::cout << "Good choice: c5 - Sicilian Defense\n";
        } else {
            std::cout << "Move chosen: " << best_move_notation << " - appears reasonable\n";
        }
    }
    
    std::cout << "\nFinal Assessment: ";
    if (best_move_notation != "f7f6" && best_move_notation.find("e8") == std::string::npos) {
        std::cout << "King safety evaluation working correctly! 🎉\n";
    } else {
        std::cout << "More work needed on evaluation system. ⚠️\n";
    }
    
    return 0;
}
