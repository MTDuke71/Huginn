#include "src/evaluation.hpp"
#include <iostream>

int main() {
    // Test our endgame threshold calculation
    int white_material = 500;  // 1 rook
    int black_material = 500;  // 1 rook  
    int total_material = white_material + black_material;
    
    std::cout << "White material: " << white_material << std::endl;
    std::cout << "Black material: " << black_material << std::endl; 
    std::cout << "Total material: " << total_material << std::endl;
    std::cout << "Endgame threshold: " << EvalParams::ENDGAME_MATERIAL_THRESHOLD << std::endl;
    std::cout << "Endgame threshold * 2: " << (EvalParams::ENDGAME_MATERIAL_THRESHOLD * 2) << std::endl;
    
    bool is_endgame_current = (total_material <= EvalParams::ENDGAME_MATERIAL_THRESHOLD * 2);
    bool is_endgame_no_mult = (total_material <= EvalParams::ENDGAME_MATERIAL_THRESHOLD);
    
    std::cout << "Is endgame (current logic): " << is_endgame_current << std::endl;
    std::cout << "Is endgame (no *2): " << is_endgame_no_mult << std::endl;
    
    // Check king table values
    std::cout << "\nKing table values:" << std::endl;
    std::cout << "King opening e1 (sq28): " << EvalParams::KING_TABLE[28] << std::endl;
    std::cout << "King endgame e1 (sq28): " << EvalParams::KING_TABLE_ENDGAME[28] << std::endl;
    std::cout << "King opening e8 (sq4): " << EvalParams::KING_TABLE[4] << std::endl;
    std::cout << "King endgame e8 (sq4): " << EvalParams::KING_TABLE_ENDGAME[4] << std::endl;
    
    return 0;
}
