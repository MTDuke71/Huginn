#include <iostream>
#include <string>
#include <iomanip>
#include "src/position.hpp"
#include "src/minimal_search.hpp"

using namespace Huginn;

int main() {
    MinimalEngine engine;
    
    // The failing position from debug_mirror_eval
    std::string fen1 = "1qr3k1/p2nbppp/bp2p3/3p4/3P4/1P2PNP1/P2Q1PBP/1N2R1K1 b - - 0 1";
    std::string fen2 = "1n2r1k1/p2q1pbp/1p2pnp1/3p4/3P4/BP2P3/P2NBPPP/1QR3K1 w - - 0 1";
    
    Position pos1, pos2;
    pos1.set_from_fen(fen1);
    pos2.set_from_fen(fen2);
    
    std::cout << "========================================" << std::endl;
    std::cout << "SIMPLE EVALUATION DEBUG" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::cout << "\nPosition 1 FEN: " << fen1 << std::endl;
    std::cout << "Side to move: " << (pos1.side_to_move == Color::White ? "White" : "Black") << std::endl;
    int eval1 = engine.evaluate(pos1);
    std::cout << "Evaluation: " << eval1 << " cp" << std::endl;
    
    std::cout << "\nPosition 2 FEN: " << fen2 << std::endl;
    std::cout << "Side to move: " << (pos2.side_to_move == Color::White ? "White" : "Black") << std::endl;
    int eval2 = engine.evaluate(pos2);
    std::cout << "Evaluation: " << eval2 << " cp" << std::endl;
    
    std::cout << "\n--- ANALYSIS ---" << std::endl;
    std::cout << "Position 1 eval: " << eval1 << " cp (Black to move)" << std::endl;
    std::cout << "Position 2 eval: " << eval2 << " cp (White to move)" << std::endl;
    std::cout << "Direct difference: " << (eval1 - eval2) << " cp" << std::endl;
    
    // For true comparison, we need to compare both from same side's perspective
    std::cout << "\n--- CORRECTED COMPARISON ---" << std::endl;
    std::cout << "Both evaluations from Black's perspective:" << std::endl;
    std::cout << "Position 1: " << eval1 << " cp" << std::endl;
    std::cout << "Position 2: " << (-eval2) << " cp" << std::endl;
    std::cout << "Difference: " << (eval1 - (-eval2)) << " cp" << std::endl;
    
    // Now test the engine's MirrorAvailTest function
    std::cout << "\n--- ENGINE MIRROR TEST ---" << std::endl;
    engine.MirrorAvailTest(pos1);
    
    return 0;
}
