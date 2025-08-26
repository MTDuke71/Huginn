#include "evaluation.hpp"
#include "position.hpp"
#include <iostream>

int main() {
    std::cout << "Detailed e2-e4 Analysis\n";
    std::cout << "========================\n\n";
    
    // Starting position
    Position start_pos;
    start_pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Starting position:\n";
    std::cout << "Material: " << Evaluation::evaluate_material(start_pos) << "cp\n";
    std::cout << "Positional: " << Evaluation::evaluate_positional(start_pos) << "cp\n";
    std::cout << "King Safety (White): " << Evaluation::evaluate_king_safety(start_pos, Color::White) << "cp\n";
    std::cout << "King Safety (Black): " << Evaluation::evaluate_king_safety(start_pos, Color::Black) << "cp\n";
    std::cout << "Pawn Structure: " << Evaluation::evaluate_pawn_structure(start_pos) << "cp\n";
    std::cout << "Development: " << Evaluation::evaluate_development(start_pos) << "cp\n";
    std::cout << "Total: " << Evaluation::evaluate_position(start_pos) << "cp\n\n";
    
    // Position after e2-e4
    Position after_e4;
    after_e4.set_from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    
    std::cout << "After e2-e4:\n";
    std::cout << "Material: " << Evaluation::evaluate_material(after_e4) << "cp\n";
    std::cout << "Positional: " << Evaluation::evaluate_positional(after_e4) << "cp\n";
    std::cout << "King Safety (White): " << Evaluation::evaluate_king_safety(after_e4, Color::White) << "cp\n";
    std::cout << "King Safety (Black): " << Evaluation::evaluate_king_safety(after_e4, Color::Black) << "cp\n";
    std::cout << "Pawn Structure: " << Evaluation::evaluate_pawn_structure(after_e4) << "cp\n";
    std::cout << "Development: " << Evaluation::evaluate_development(after_e4) << "cp\n";
    std::cout << "Total: " << Evaluation::evaluate_position(after_e4) << "cp\n\n";
    
    std::cout << "Changes:\n";
    std::cout << "========\n";
    int material_change = Evaluation::evaluate_material(after_e4) - Evaluation::evaluate_material(start_pos);
    int positional_change = Evaluation::evaluate_positional(after_e4) - Evaluation::evaluate_positional(start_pos);
    int king_safety_w_change = Evaluation::evaluate_king_safety(after_e4, Color::White) - Evaluation::evaluate_king_safety(start_pos, Color::White);
    int king_safety_b_change = Evaluation::evaluate_king_safety(after_e4, Color::Black) - Evaluation::evaluate_king_safety(start_pos, Color::Black);
    int pawn_structure_change = Evaluation::evaluate_pawn_structure(after_e4) - Evaluation::evaluate_pawn_structure(start_pos);
    int development_change = Evaluation::evaluate_development(after_e4) - Evaluation::evaluate_development(start_pos);
    
    std::cout << "Material change: " << material_change << "cp\n";
    std::cout << "Positional change: " << positional_change << "cp\n"; 
    std::cout << "King Safety (White) change: " << king_safety_w_change << "cp\n";
    std::cout << "King Safety (Black) change: " << king_safety_b_change << "cp\n";
    std::cout << "Pawn Structure change: " << pawn_structure_change << "cp\n";
    std::cout << "Development change: " << development_change << "cp\n";
    std::cout << "Total change: " << (material_change + positional_change + king_safety_w_change + king_safety_b_change + pawn_structure_change + development_change) << "cp\n";
    
    return 0;
}
