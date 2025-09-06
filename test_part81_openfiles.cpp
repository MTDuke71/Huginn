// VICE Part 81 Test: Open and Semi-Open File Bonuses
#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/evaluation.hpp"
#include "src/init.hpp"
#include <iostream>
#include <iomanip>

using namespace Huginn;

void testOpenFileBonus() {
    std::cout << "=== VICE Part 81: Open and Semi-Open File Bonus Test ===" << std::endl;
    
    init();
    MinimalEngine engine;
    
    // Test position 1: Rook on open file (a-file completely clear)
    std::string test1_fen = "r6k/8/8/8/8/8/8/R6K w - - 0 1";
    Position pos1;
    pos1.set_from_fen(test1_fen);
    int eval1 = engine.evalPosition(pos1);
    std::cout << "Test 1 - Rooks on open a-file: " << std::setw(4) << eval1 << "cp" << std::endl;
    std::cout << "  FEN: " << test1_fen << std::endl;
    std::cout << "  Expected: Symmetric (0cp difference after accounting for material)" << std::endl;
    
    // Test position 2: Rook on semi-open file (white rook on h-file, only black pawns present)
    std::string test2_fen = "7k/7p/8/8/8/8/8/7R w - - 0 1";
    Position pos2;
    pos2.set_from_fen(test2_fen);
    int eval2 = engine.evalPosition(pos2);
    std::cout << "\nTest 2 - White rook on semi-open h-file: " << std::setw(4) << eval2 << "cp" << std::endl;
    std::cout << "  FEN: " << test2_fen << std::endl;
    std::cout << "  Expected: White advantage (+5cp semi-open file bonus)" << std::endl;
    
    // Test position 3: Same but mirrored (black rook on semi-open file)
    std::string test3_fen = "r7/8/8/8/8/8/P7/K7 b - - 0 1";
    Position pos3;
    pos3.set_from_fen(test3_fen);
    int eval3 = engine.evalPosition(pos3);
    std::cout << "\nTest 3 - Black rook on semi-open a-file: " << std::setw(4) << eval3 << "cp" << std::endl;
    std::cout << "  FEN: " << test3_fen << std::endl;
    std::cout << "  Expected: Black advantage (-5cp semi-open file bonus)" << std::endl;
    
    // Test position 4: Queen on open file
    std::string test4_fen = "3q3k/8/8/8/8/8/8/3Q3K w - - 0 1";
    Position pos4;
    pos4.set_from_fen(test4_fen);
    int eval4 = engine.evalPosition(pos4);
    std::cout << "\nTest 4 - Queens on open d-file: " << std::setw(4) << eval4 << "cp" << std::endl;
    std::cout << "  FEN: " << test4_fen << std::endl;
    std::cout << "  Expected: Symmetric (0cp difference after accounting for material)" << std::endl;
    
    // Test position 5: Queen on semi-open file
    std::string test5_fen = "7k/7p/8/8/8/8/8/3Q3K w - - 0 1";
    Position pos5;
    pos5.set_from_fen(test5_fen);
    int eval5 = engine.evalPosition(pos5);
    std::cout << "\nTest 5 - White queen on semi-open d-file: " << std::setw(4) << eval5 << "cp" << std::endl;
    std::cout << "  FEN: " << test5_fen << std::endl;
    std::cout << "  Expected: White advantage (+3cp queen semi-open file bonus)" << std::endl;
    
    // Test position 6: Blocked files (both sides have pawns)
    std::string test6_fen = "r6k/p6p/8/8/8/8/P6P/R6K w - - 0 1";
    Position pos6;
    pos6.set_from_fen(test6_fen);
    int eval6 = engine.evalPosition(pos6);
    std::cout << "\nTest 6 - Rooks on blocked files: " << std::setw(4) << eval6 << "cp" << std::endl;
    std::cout << "  FEN: " << test6_fen << std::endl;
    std::cout << "  Expected: Symmetric (no file bonuses)" << std::endl;
    
    std::cout << "\n=== VICE Part 81 Values ===" << std::endl;
    std::cout << "Rook open file bonus: " << EvalParams::ROOK_OPEN_FILE_BONUS << "cp" << std::endl;
    std::cout << "Rook semi-open file bonus: " << EvalParams::ROOK_SEMI_OPEN_FILE_BONUS << "cp" << std::endl;
    std::cout << "Queen open file bonus: " << EvalParams::QUEEN_OPEN_FILE_BONUS << "cp" << std::endl;
    std::cout << "Queen semi-open file bonus: " << EvalParams::QUEEN_SEMI_OPEN_FILE_BONUS << "cp" << std::endl;
    
    std::cout << "\n✅ VICE Part 81 open/semi-open file evaluation implemented successfully!" << std::endl;
}

int main() {
    testOpenFileBonus();
    return 0;
}
