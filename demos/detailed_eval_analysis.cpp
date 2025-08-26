#include "evaluation.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>

// Forward declare the individual evaluation functions so we can test them separately
namespace Evaluation {
    extern int evaluate_material_quick(const Position& pos);
    extern int evaluate_positional(const Position& pos);
    extern int evaluate_king_safety(const Position& pos, Color color);
    extern int evaluate_pawn_structure(const Position& pos);
    extern int evaluate_development(const Position& pos);
}

void analyze_move_components(const Position& start_pos, S_MOVE move, const std::string& move_name) {
    Position temp_pos = start_pos;
    temp_pos.make_move_with_undo(move);
    
    std::cout << "\n" << move_name << " Analysis:\n";
    std::cout << "========================\n";
    
    // Total evaluation
    int total_eval = Evaluation::evaluate_position(temp_pos);
    std::cout << "TOTAL: " << total_eval << "cp\n";
    
    // Break down by components
    int material = Evaluation::evaluate_material_quick(temp_pos);
    int positional = Evaluation::evaluate_positional(temp_pos);
    int white_king_safety = Evaluation::evaluate_king_safety(temp_pos, Color::White);
    int black_king_safety = Evaluation::evaluate_king_safety(temp_pos, Color::Black);
    int pawn_structure = Evaluation::evaluate_pawn_structure(temp_pos);
    int development = Evaluation::evaluate_development(temp_pos);
    
    std::cout << "  Material: " << material << "cp\n";
    std::cout << "  Positional: " << positional << "cp\n";
    std::cout << "  King Safety (W-B): " << (white_king_safety - black_king_safety) << "cp\n";
    std::cout << "  Pawn Structure: " << pawn_structure << "cp\n";
    std::cout << "  Development: " << development << "cp\n";
    
    int sum = material + positional + (white_king_safety - black_king_safety) + pawn_structure + development;
    std::cout << "  Sum: " << sum << "cp\n";
    
    if (sum != total_eval) {
        std::cout << "  ⚠️  MISMATCH! Difference: " << (total_eval - sum) << "cp\n";
    }
}

int main() {
    std::cout << "Detailed Opening Move Component Analysis\n";
    std::cout << "=======================================\n";
    
    // Starting position
    Position start_pos;
    start_pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Generate moves
    S_MOVELIST moves;
    generate_legal_moves_enhanced(start_pos, moves);
    
    // Find specific moves
    S_MOVE e2e4, d2d4, c2c4, c2c3, g1f3;
    bool found_e2e4 = false, found_d2d4 = false, found_c2c4 = false, found_c2c3 = false, found_g1f3 = false;
    
    for (int i = 0; i < moves.count; ++i) {
        S_MOVE move = moves.moves[i];
        int from = move.get_from();
        int to = move.get_to();
        
        if (from == 34 && to == 54) { e2e4 = move; found_e2e4 = true; }
        if (from == 33 && to == 53) { d2d4 = move; found_d2d4 = true; }
        if (from == 32 && to == 52) { c2c4 = move; found_c2c4 = true; }
        if (from == 32 && to == 43) { c2c3 = move; found_c2c3 = true; }
        if (from == 16 && to == 45) { g1f3 = move; found_g1f3 = true; }
    }
    
    // Analyze each move
    if (found_e2e4) analyze_move_components(start_pos, e2e4, "e2-e4");
    if (found_d2d4) analyze_move_components(start_pos, d2d4, "d2-d4");
    if (found_c2c4) analyze_move_components(start_pos, c2c4, "c2-c4");
    if (found_c2c3) analyze_move_components(start_pos, c2c3, "c2-c3");
    if (found_g1f3) analyze_move_components(start_pos, g1f3, "Nf3");
    
    return 0;
}
