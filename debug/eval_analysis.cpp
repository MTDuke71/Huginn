#include "../src/position.hpp"
#include "../src/evaluation.hpp"
#include "../src/init.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    init();
    
    std::cout << "=== EVALUATION ANALYSIS ===" << std::endl;
    Position pos;
    
    // Test 1: Starting position
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::cout << "Starting position: " << HybridEvaluator::evaluate(pos) << std::endl;
    
    // Test 2: After developing knight vs pawn move
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    std::cout << "After 1.e4 (from Black's view): " << HybridEvaluator::evaluate(pos) << std::endl;
    
    // Test 3: Black develops knight (good)
    pos.set_from_fen("rnbqkb1r/pppppppp/5n2/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 1 2");
    std::cout << "After 1.e4 Nf6 (from White's view): " << HybridEvaluator::evaluate(pos) << std::endl;
    
    // Test 4: Black plays pawn move instead (bad)
    pos.set_from_fen("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
    std::cout << "After 1.e4 e5 (from White's view): " << HybridEvaluator::evaluate(pos) << std::endl;
    
    // Test 5: Position where Huginn played too many pawn moves
    pos.set_from_fen("rnbqkbnr/1p1ppppp/p7/1p6/2P1P3/8/PP1P1PPP/RNBQKBNR w KQkq b6 0 4");
    int eval1 = HybridEvaluator::evaluate(pos);
    std::cout << "After 1.c4 b6 2.e4 a6 3.d4 b5 (White to move): " << eval1 << std::endl;
    
    // Test 6: Better development for Black
    pos.set_from_fen("r1bqkbnr/pppppppp/2n5/8/2P1P3/8/PP1P1PPP/RNBQKBNR w KQkq - 2 3");
    int eval2 = HybridEvaluator::evaluate(pos);
    std::cout << "If Black had played 1...Nc6 instead: " << eval2 << std::endl;
    std::cout << "Development advantage: " << (eval2 - eval1) << " centipawns" << std::endl;
    
    // Check game phases
    GamePhase phase = HybridEvaluator::get_game_phase(pos);
    std::cout << "Current game phase: " << (int)phase << " (0=Opening, 1=Middlegame, 2=Endgame)" << std::endl;
    
    return 0;
}
