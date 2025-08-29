#include <iostream>
#include <vector>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "init.hpp"

int main() {
    // Initialize zobrist hash tables
    Zobrist::init_zobrist();
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "Testing VICE Tutorial Video #41: MakeMove function\n";
    std::cout << "=================================================\n\n";
    
    // Test 1: Legal move (e2-e4)
    std::cout << "Test 1: Legal move e2-e4\n";
    S_MOVE move1(sq(File::E, Rank::R2), sq(File::E, Rank::R4), PieceType::None, false, true, PieceType::None, false);
    
    std::cout << "Before move: Side to move = " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "Before move: Ply = " << pos.ply << std::endl;
    
    int result1 = pos.MakeMove(move1);
    std::cout << "MakeMove result: " << result1 << " (1 = legal, 0 = illegal)" << std::endl;
    std::cout << "After move: Side to move = " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "After move: Ply = " << pos.ply << std::endl;
    std::cout << "En passant square: " << pos.ep_square << std::endl;
    
    // Test 2: Try another legal move (g8-f6)
    std::cout << "\nTest 2: Legal move Ng8-f6\n"; 
    S_MOVE move2(sq(File::G, Rank::R8), sq(File::F, Rank::R6), PieceType::None, false, false, PieceType::None, false);
    
    int result2 = pos.MakeMove(move2);
    std::cout << "MakeMove result: " << result2 << " (1 = legal, 0 = illegal)" << std::endl;
    std::cout << "After move: Side to move = " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "After move: Ply = " << pos.ply << std::endl;
    
    // Test 3: Try an illegal move that would leave king in check
    std::cout << "\nTest 3: Trying to create an illegal position\n";
    pos.set_from_fen("rnbqkb1r/pppp1ppp/5n2/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 2 3");
    std::cout << "Set position to test illegal move (king would be in check)\n";
    
    // Try to move a piece that would expose the king to check
    S_MOVE illegal_move(sq(File::F, Rank::R2), sq(File::F, Rank::R3), PieceType::None, false, false, PieceType::None, false);
    
    std::cout << "Before illegal move: Ply = " << pos.ply << std::endl;
    int result3 = pos.MakeMove(illegal_move);
    std::cout << "MakeMove result: " << result3 << " (should be 0 for illegal)" << std::endl;
    std::cout << "After illegal move attempt: Ply = " << pos.ply << " (should be same as before)" << std::endl;
    
    // Test 4: Castling move
    std::cout << "\nTest 4: Castling move\n";
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    std::cout << "Set position for castling test\n";
    
    S_MOVE castling_move(sq(File::E, Rank::R1), sq(File::G, Rank::R1), PieceType::None, false, false, PieceType::None, true);
    int result4 = pos.MakeMove(castling_move);
    std::cout << "White kingside castling result: " << result4 << std::endl;
    
    std::cout << "\nAll MakeMove tests completed!\n";
    
    return 0;
}
