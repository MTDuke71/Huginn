// Debug king square values
#include <iostream>
#include "src/evaluation.hpp"

using namespace Huginn;

int main() {
    std::cout << "=== King Table Values Debug ===" << std::endl;
    
    // e1 = file 4, rank 0 = square 4
    // e4 = file 4, rank 3 = square 28
    
    std::cout << "Square mappings:" << std::endl;
    std::cout << "e1 (file 4, rank 0) = square " << (0 * 8 + 4) << std::endl;
    std::cout << "e4 (file 4, rank 3) = square " << (3 * 8 + 4) << std::endl;
    
    std::cout << "\nOpening table values:" << std::endl;
    std::cout << "e1 (square 4): " << EvalParams::KING_TABLE[4] << " cp" << std::endl;
    std::cout << "e4 (square 28): " << EvalParams::KING_TABLE[28] << " cp" << std::endl;
    std::cout << "Difference (e4 - e1): " << (EvalParams::KING_TABLE[28] - EvalParams::KING_TABLE[4]) << " cp" << std::endl;
    
    std::cout << "\nEndgame table values:" << std::endl;
    std::cout << "e1 (square 4): " << EvalParams::KING_TABLE_ENDGAME[4] << " cp" << std::endl;
    std::cout << "e4 (square 28): " << EvalParams::KING_TABLE_ENDGAME[28] << " cp" << std::endl;
    std::cout << "Difference (e4 - e1): " << (EvalParams::KING_TABLE_ENDGAME[28] - EvalParams::KING_TABLE_ENDGAME[4]) << " cp" << std::endl;
    
    // Also check a1 (0) and d4 (27)
    std::cout << "\nTable comparison (a1 vs d4):" << std::endl;
    std::cout << "a1 opening: " << EvalParams::KING_TABLE[0] << " cp" << std::endl;
    std::cout << "d4 opening: " << EvalParams::KING_TABLE[27] << " cp" << std::endl;
    std::cout << "a1 endgame: " << EvalParams::KING_TABLE_ENDGAME[0] << " cp" << std::endl;
    std::cout << "d4 endgame: " << EvalParams::KING_TABLE_ENDGAME[27] << " cp" << std::endl;
    
    return 0;
}
