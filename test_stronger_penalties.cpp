#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/init.hpp"
#include "src/evaluation.hpp"
#include <iostream>

int main() {
    AllInit();
    
    Position pos;
    
    std::cout << "Testing MUCH STRONGER Opening Principle Penalties\n";
    std::cout << "================================================\n\n";
    
    // Starting position
    ParseFEN(&pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::cout << "Starting position: " << EvalPosition(&pos) << "cp\n";
    
    // Good opening: 1.e4
    ParseFEN(&pos, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    std::cout << "After 1.e4 (GOOD): " << EvalPosition(&pos) << "cp\n";
    
    // Bad opening from the game: 1.e3 f5 2.h3 d5 3.d3 c6 4.a3
    ParseFEN(&pos, "rnbqkb1r/pp2pppp/2p5/3p1n2/8/P2PP2P/1PP2PP1/RNBQKBNR w KQkq - 0 5");
    std::cout << "After 1.e3 f5 2.h3 d5 3.d3 c6 4.a3 (TERRIBLE): " << EvalPosition(&pos) << "cp\n";
    
    // Even worse - position after many pawn moves (like move 10 from game)
    ParseFEN(&pos, "rnbqkbnr/1p4p1/2p1p1p1/p1pp4/P2P1P1P/1P1P4/2P3P1/RNBQKBNR w KQkq - 0 11");
    std::cout << "After 10 pawn moves each (HORRIFIC): " << EvalPosition(&pos) << "cp\n";
    
    std::cout << "\nNOW testing if e4 gets MASSIVE bonus over e3:\n";
    
    // 1.e4 - should get +100cp center bonus + development encouragement  
    ParseFEN(&pos, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    std::cout << "1.e4: " << EvalPosition(&pos) << "cp\n";
    
    // 1.e3 - should get only +20cp center bonus
    ParseFEN(&pos, "rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    std::cout << "1.e3: " << EvalPosition(&pos) << "cp\n";
    
    std::cout << "\nDifference should be 80cp in favor of e4!\n";
    
    return 0;
}
