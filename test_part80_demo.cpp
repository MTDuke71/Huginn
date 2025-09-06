// VICE Part 80 Demo: Isolated Pawn Penalties and Passed Pawn Bonuses
// This demonstrates the enhanced evaluation function with symmetry testing

#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/init.hpp"
#include <iostream>

using namespace Huginn;

void print_fen_and_eval(const std::string& description, const std::string& fen, MinimalEngine& engine) {
    std::cout << "\n" << description << std::endl;
    std::cout << "FEN: " << fen << std::endl;
    
    Position pos;
    pos.set_from_fen(fen);
    std::cout << "Evaluation: " << engine.evalPosition(pos) << " cp" << std::endl;
    engine.MirrorAvailTest(pos);
}

int main() {
    std::cout << "=== VICE Part 80: Enhanced Evaluation Demo ===" << std::endl;
    
    // Initialize the engine
    init();
    
    MinimalEngine engine;
    
    print_fen_and_eval("1. Starting Position - Mirror Test",
                      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                      engine);
    
    print_fen_and_eval("2. Position with Isolated Pawns",
                      "r1bqkb1r/ppp1pppp/2n2n2/3p4/3P4/2N2N2/PPP1PPPP/R1BQKB1R w KQkq - 4 4",
                      engine);
    
    print_fen_and_eval("3. Position with Passed Pawns", 
                      "r3k2r/pp3ppp/2n5/2bp4/2nP4/2P5/PP3PPP/R3K2R w KQkq - 0 10",
                      engine);
    
    print_fen_and_eval("4. Complex Pawn Structure",
                      "r3k2r/p1p2ppp/1p6/3pP3/1P1P4/8/P1P2PPP/R3K2R w KQkq - 0 15",
                      engine);
    
    print_fen_and_eval("5. Endgame with Advanced Passed Pawns",
                      "8/2k5/8/3pP3/1P6/8/2K5/8 w - - 0 40",
                      engine);
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    std::cout << "Features demonstrated:" << std::endl;
    std::cout << "✓ MirrorAvailTest function for debugging evaluation symmetry" << std::endl;
    std::cout << "✓ Isolated pawn penalties using pre-computed masks" << std::endl;
    std::cout << "✓ Passed pawn bonuses with rank-based scaling" << std::endl;
    std::cout << "✓ Bitboard-based efficient pawn structure analysis" << std::endl;
    
    return 0;
}
