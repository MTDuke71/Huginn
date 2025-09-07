#include <iostream>
#include <string>
#include <iomanip>
#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/evaluation.hpp"

using namespace Huginn;

int main() {
    MinimalEngine engine;
    
    // Position 1 from the mirror test results
    std::string fen1 = "1qr3k1/p2nbppp/bp2p3/3p4/3P4/1P2PNP1/P2Q1PBP/1N2R1K1 b - - 0 1";
    std::string fen2 = "1n2r1k1/p2q1pbp/1p2pnp1/3p4/3P4/BP2P3/P2NBPPP/1QR3K1 w - - 0 1";
    
    Position pos1, pos2;
    pos1.set_from_fen(fen1);
    pos2.set_from_fen(fen2);
    
    std::cout << "========================================" << std::endl;
    std::cout << "POSITION 1 (Original)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "FEN: " << fen1 << std::endl;
    std::cout << "Side to move: Black" << std::endl;
    int eval1 = engine.evaluate(pos1);
    std::cout << "Final evaluation: " << eval1 << " cp" << std::endl;
    
    std::cout << "\n\n========================================" << std::endl;
    std::cout << "POSITION 2 (Mirrored)" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "FEN: " << fen2 << std::endl;
    std::cout << "Side to move: White" << std::endl;
    int eval2 = engine.evaluate(pos2);
    std::cout << "Final evaluation: " << eval2 << " cp" << std::endl;
    
    std::cout << "\n\n========================================" << std::endl;
    std::cout << "COMPARISON ANALYSIS" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Position 1 final: " << eval1 << " cp" << std::endl;
    std::cout << "Position 2 final: " << eval2 << " cp" << std::endl;
    std::cout << "Difference: " << (eval1 - eval2) << " cp" << std::endl;
    std::cout << "Expected: 0 cp (for perfect symmetry)" << std::endl;
    
    // Let's also test the mirror function from the engine
    std::cout << "\n--- USING ENGINE'S MIRROR TEST ---" << std::endl;
    engine.MirrorAvailTest(pos1);
    
    return 0;
}
