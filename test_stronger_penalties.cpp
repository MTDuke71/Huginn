#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include <iostream>

int main() {
    InitializeTables();
    
    MinimalEngine engine;
    Position pos;
    
    std::cout << "Testing MUCH STRONGER Opening Principle Penalties\n";
    std::cout << "================================================\n\n";
    
    // Starting position
    pos.parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::cout << "Starting position: " << engine.evaluate(pos) << "cp\n";
    
    // Good opening: 1.e4
    pos.parse_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    std::cout << "After 1.e4 (GOOD): " << engine.evaluate(pos) << "cp\n";
    
    // Bad opening from the game: 1.e3 f5 2.h3 d5 3.d3 c6 4.a3
    pos.parse_fen("rnbqkb1r/pp2pppp/2p5/3p1n2/8/P2PP2P/1PP2PP1/RNBQKBNR w KQkq - 0 5");
    std::cout << "After 1.e3 f5 2.h3 d5 3.d3 c6 4.a3 (TERRIBLE): " << engine.evaluate(pos) << "cp\n";
    
    // Even worse - position after many pawn moves (like move 10 from game)
    pos.parse_fen("rnbqkbnr/1p4p1/2p1p1p1/p1pp4/P2P1P1P/1P1P4/2P3P1/RNBQKBNR w KQkq - 0 11");
    std::cout << "After 10 pawn moves each (HORRIFIC): " << engine.evaluate(pos) << "cp\n";
    
    std::cout << "\nNOW testing if e4 gets MASSIVE bonus over e3:\n";
    
    // 1.e4 - should get +100cp center bonus + development encouragement  
    pos.parse_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    std::cout << "1.e4: " << engine.evaluate(pos) << "cp\n";
    
    // 1.e3 - should get only +20cp center bonus
    pos.parse_fen("rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    std::cout << "1.e3: " << engine.evaluate(pos) << "cp\n";
    
    std::cout << "\nDifference should be 80cp in favor of e4!\n";
    
    return 0;
}
