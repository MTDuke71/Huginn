#include <iostream>
#include "position.hpp"
#include "init.hpp"

int main() {
    Huginn::init();
    Position pos;
    pos.set_startpos();
    
    std::cout << "=== Material Calculation Verification ===" << std::endl;
    std::cout << "Starting position material per side: " << pos.get_material_score(Color::White) << std::endl;
    std::cout << "Expected: 8*100 + 2*500 + 2*320 + 2*330 + 1*900 = " << (8*100 + 2*500 + 2*320 + 2*330 + 1*900) << std::endl;
    std::cout << "Calculation: 800 + 1000 + 640 + 660 + 900 = " << (800 + 1000 + 640 + 660 + 900) << std::endl;
    
    // King and pawn endgame
    Position endgame;
    endgame.set_from_fen("8/8/8/3k4/3P4/3K4/8/8 w - - 0 1");
    std::cout << "\nKing and pawn endgame:" << std::endl;
    std::cout << "White material: " << endgame.get_material_score(Color::White) << " (should be 100)" << std::endl;
    std::cout << "Black material: " << endgame.get_material_score(Color::Black) << " (should be 0)" << std::endl;
    std::cout << "Balance: " << endgame.get_material_balance() << " (should be 100)" << std::endl;
    
    return 0;
}
