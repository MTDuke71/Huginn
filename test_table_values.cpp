#include "src/evaluation.hpp"
#include <iostream>

int main() {
    std::cout << "=== King Table Values ===" << std::endl;
    
    // e1 = index 4
    std::cout << "Opening table e1 (index 4): " << EvalParams::KING_TABLE[4] << std::endl;
    std::cout << "Endgame table e1 (index 4): " << EvalParams::KING_TABLE_ENDGAME[4] << std::endl;
    
    // e8 = index 60
    std::cout << "Opening table e8 (index 60): " << EvalParams::KING_TABLE[60] << std::endl;  
    std::cout << "Endgame table e8 (index 60): " << EvalParams::KING_TABLE_ENDGAME[60] << std::endl;
    
    // Also check our material calculation logic
    std::cout << "\n=== Material Calculation ===" << std::endl;
    int white_material = 500;  // 1 rook
    int black_material = 500;  // 1 rook  
    int total_material = white_material + black_material;
    
    bool is_endgame = (total_material <= EvalParams::ENDGAME_MATERIAL_THRESHOLD);
    std::cout << "Total material: " << total_material << std::endl;
    std::cout << "Endgame threshold: " << EvalParams::ENDGAME_MATERIAL_THRESHOLD << std::endl;
    std::cout << "Is endgame: " << is_endgame << std::endl;
    
    return 0;
}
