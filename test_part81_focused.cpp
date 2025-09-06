// VICE Part 81 Simple Test: Focus on file bonuses only
#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/evaluation.hpp"
#include "src/init.hpp"
#include <iostream>
#include <iomanip>

using namespace Huginn;

void testFileBonusesFocused() {
    std::cout << "=== VICE Part 81: File Bonus Focused Test ===" << std::endl;
    
    init();
    MinimalEngine engine;
    
    // Test 1: Compare rook on open file vs closed file
    std::string pos1_open = "4k3/8/8/8/8/8/8/R3K3 w - - 0 1";    // Rook on open a-file
    std::string pos1_closed = "4k3/8/8/8/8/8/p7/R3K3 w - - 0 1";  // Rook on file with pawn
    
    Position p1_open, p1_closed;
    p1_open.set_from_fen(pos1_open);
    p1_closed.set_from_fen(pos1_closed);
    
    int eval_open = engine.evalPosition(p1_open);
    int eval_closed = engine.evalPosition(p1_closed);
    int diff1 = eval_open - eval_closed;
    
    std::cout << "Test 1 - Rook open vs closed file:" << std::endl;
    std::cout << "  Open file:   " << std::setw(4) << eval_open << "cp" << std::endl;
    std::cout << "  Closed file: " << std::setw(4) << eval_closed << "cp" << std::endl;
    std::cout << "  Difference:  " << std::setw(4) << diff1 << "cp (Expected: +10cp for open file)" << std::endl;
    
    // Test 2: Compare rook on semi-open vs closed file
    std::string pos2_semi = "4k3/8/8/8/8/8/8/R3K3 w - - 0 1";    // Rook on semi-open a-file (no white pawns)
    std::string pos2_white_pawn = "4k3/8/8/8/8/8/P7/R3K3 w - - 0 1"; // Rook on file with white pawn
    
    Position p2_semi, p2_white_pawn;
    p2_semi.set_from_fen(pos2_semi);
    p2_white_pawn.set_from_fen(pos2_white_pawn);
    
    int eval_semi = engine.evalPosition(p2_semi);
    int eval_white_pawn = engine.evalPosition(p2_white_pawn);
    int diff2 = eval_semi - eval_white_pawn;
    
    std::cout << "\nTest 2 - Rook semi-open vs own pawn:" << std::endl;
    std::cout << "  Semi-open:   " << std::setw(4) << eval_semi << "cp" << std::endl;
    std::cout << "  With own pawn:" << std::setw(4) << eval_white_pawn << "cp" << std::endl;
    std::cout << "  Difference:  " << std::setw(4) << diff2 << "cp (Expected: +5cp for semi-open)" << std::endl;
    
    // Test 3: Queen on open vs closed file
    std::string pos3_open = "4k3/8/8/8/8/8/8/Q3K3 w - - 0 1";    // Queen on open a-file
    std::string pos3_closed = "4k3/8/8/8/8/8/p7/Q3K3 w - - 0 1";  // Queen on file with pawn
    
    Position p3_open, p3_closed;
    p3_open.set_from_fen(pos3_open);
    p3_closed.set_from_fen(pos3_closed);
    
    int eval3_open = engine.evalPosition(p3_open);
    int eval3_closed = engine.evalPosition(p3_closed);
    int diff3 = eval3_open - eval3_closed;
    
    std::cout << "\nTest 3 - Queen open vs closed file:" << std::endl;
    std::cout << "  Open file:   " << std::setw(4) << eval3_open << "cp" << std::endl;
    std::cout << "  Closed file: " << std::setw(4) << eval3_closed << "cp" << std::endl;
    std::cout << "  Difference:  " << std::setw(4) << diff3 << "cp (Expected: +5cp for open file)" << std::endl;
    
    std::cout << "\n=== Expected Bonuses ===" << std::endl;
    std::cout << "Rook open file: +" << EvalParams::ROOK_OPEN_FILE_BONUS << "cp" << std::endl;
    std::cout << "Rook semi-open: +" << EvalParams::ROOK_SEMI_OPEN_FILE_BONUS << "cp" << std::endl;
    std::cout << "Queen open file: +" << EvalParams::QUEEN_OPEN_FILE_BONUS << "cp" << std::endl;
    std::cout << "Queen semi-open: +" << EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS << "cp" << std::endl;
}

int main() {
    testFileBonusesFocused();
    return 0;
}
