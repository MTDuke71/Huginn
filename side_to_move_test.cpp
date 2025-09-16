#include "src/bitboard_position.hpp"
#include <iostream>

int main() {
    std::cout << "=== Side To Move Debug Test ===\n\n";
    
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Initial position:\n";
    std::cout << "FEN: " << pos.to_fen() << "\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n\n";
    
    // Make h2h4
    SimpleBitboardMove h2h4;
    h2h4.from_64 = 15;  // h2
    h2h4.to_64 = 31;    // h4
    h2h4.is_capture = false;
    h2h4.is_ep_capture = false;
    h2h4.is_castling = false;
    h2h4.is_promotion = false;
    h2h4.promotion_type = PieceType::None;
    
    std::cout << "Making move h2h4...\n";
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(h2h4);
    
    std::cout << "After h2h4:\n";
    std::cout << "FEN: " << pos.to_fen() << "\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    
    if (pos.side_to_move != Color::Black) {
        std::cout << "ERROR: Side to move should be Black after White's move!\n";
    } else {
        std::cout << "CORRECT: Side to move is Black after White's move.\n";
    }
    
    return 0;
}