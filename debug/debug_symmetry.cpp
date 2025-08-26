#include "evaluation.hpp"
#include "position.hpp"
#include "init.hpp"
#include <iostream>

int main() {
    Huginn::init();
    
    std::cout << "Analyzing Endgame Position Symmetry Issue\n";
    std::cout << "=========================================\n\n";
    
    // Original position
    Position pos1;
    pos1.set_from_fen("8/2k1p3/3p4/3P4/3K4/8/8/8 w - - 0 1");
    
    // Mirrored position  
    Position pos2;
    pos2.set_from_fen("8/8/8/3k4/3p4/3P4/2K1P3/8 b - - 0 1");
    
    std::cout << "ORIGINAL POSITION (White to move):\n";
    std::cout << "FEN: 8/2k1p3/3p4/3P4/3K4/8/8/8 w - - 0 1\n";
    std::cout << "Material: " << Evaluation::evaluate_material(pos1) << "cp\n";
    std::cout << "Positional: " << Evaluation::evaluate_positional(pos1) << "cp\n";
    std::cout << "King Safety (White): " << Evaluation::evaluate_king_safety(pos1, Color::White) << "cp\n";
    std::cout << "King Safety (Black): " << Evaluation::evaluate_king_safety(pos1, Color::Black) << "cp\n";
    std::cout << "Pawn Structure: " << Evaluation::evaluate_pawn_structure(pos1) << "cp\n";
    std::cout << "Development: " << Evaluation::evaluate_development(pos1) << "cp\n";
    std::cout << "Total: " << Evaluation::evaluate_position(pos1) << "cp\n\n";
    
    std::cout << "MIRRORED POSITION (Black to move):\n";
    std::cout << "FEN: 8/8/8/3k4/3p4/3P4/2K1P3/8 b - - 0 1\n";
    std::cout << "Material: " << Evaluation::evaluate_material(pos2) << "cp\n";
    std::cout << "Positional: " << Evaluation::evaluate_positional(pos2) << "cp\n";
    std::cout << "King Safety (White): " << Evaluation::evaluate_king_safety(pos2, Color::White) << "cp\n";
    std::cout << "King Safety (Black): " << Evaluation::evaluate_king_safety(pos2, Color::Black) << "cp\n";
    std::cout << "Pawn Structure: " << Evaluation::evaluate_pawn_structure(pos2) << "cp\n";
    std::cout << "Development: " << Evaluation::evaluate_development(pos2) << "cp\n";
    std::cout << "Total: " << Evaluation::evaluate_position(pos2) << "cp\n\n";
    
    std::cout << "DIFFERENCES:\n";
    std::cout << "Material: " << (Evaluation::evaluate_material(pos1) - Evaluation::evaluate_material(pos2)) << "cp\n";
    std::cout << "Positional: " << (Evaluation::evaluate_positional(pos1) - Evaluation::evaluate_positional(pos2)) << "cp\n";
    std::cout << "King Safety: " << ((Evaluation::evaluate_king_safety(pos1, Color::White) - Evaluation::evaluate_king_safety(pos1, Color::Black)) - (Evaluation::evaluate_king_safety(pos2, Color::White) - Evaluation::evaluate_king_safety(pos2, Color::Black))) << "cp\n";
    std::cout << "Pawn Structure: " << (Evaluation::evaluate_pawn_structure(pos1) - Evaluation::evaluate_pawn_structure(pos2)) << "cp\n";
    std::cout << "Development: " << (Evaluation::evaluate_development(pos1) - Evaluation::evaluate_development(pos2)) << "cp\n";
    std::cout << "Total Difference: " << (Evaluation::evaluate_position(pos1) - Evaluation::evaluate_position(pos2)) << "cp\n";
    
    return 0;
}
