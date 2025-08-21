#include "src/chess_types.hpp"
#include "src/board120.hpp"
#include "src/position.hpp"
#include "src/init.hpp"
#include "src/movegen.hpp"
#include <iostream>

int main() {
    Huginn::init();
    
    Position pos;
    
    // Test case: King cannot move into check
    // King on e4, rook on d5
    pos.set_from_fen("8/8/8/3r4/4K3/8/8/8 w - - 0 1");
    
    std::cout << "Position: King on E4, Rook on D5\n";
    
    // Debug square calculations
    std::cout << "sq(File::E, Rank::R4) = " << sq(File::E, Rank::R4) << "\n";
    std::cout << "sq(File::D, Rank::R5) = " << sq(File::D, Rank::R5) << "\n";
    std::cout << "sq(File::D, Rank::R4) = " << sq(File::D, Rank::R4) << "\n";
    std::cout << "sq(File::E, Rank::R5) = " << sq(File::E, Rank::R5) << "\n";
    std::cout << "sq(File::F, Rank::R4) = " << sq(File::F, Rank::R4) << "\n";
    std::cout << "sq(File::E, Rank::R3) = " << sq(File::E, Rank::R3) << "\n";
    
    std::cout << "King is at square: " << pos.king_sq[0] << "\n";
    
    // Check what pieces are on key squares
    std::cout << "Piece on E4: " << (int)pos.at(sq(File::E, Rank::R4)) << "\n";
    std::cout << "Piece on D5: " << (int)pos.at(sq(File::D, Rank::R5)) << "\n";
    
    // Generate and show all pseudo-legal moves
    MoveList pseudo_moves;
    generate_pseudo_legal_moves(pos, pseudo_moves);
    std::cout << "Pseudo-legal moves:\n";
    for (size_t i = 0; i < pseudo_moves.size(); ++i) {
        const S_MOVE& move = pseudo_moves[i];
        std::cout << "  Move " << i << ": from " << move.get_from() << " to " << move.get_to() << "\n";
    }
    
    // Generate legal moves
    MoveList legal_moves;
    generate_legal_moves(pos, legal_moves);
    std::cout << "Legal moves:\n";
    for (size_t i = 0; i < legal_moves.size(); ++i) {
        const S_MOVE& move = legal_moves[i];
        std::cout << "  Legal move " << i << ": from " << move.get_from() << " to " << move.get_to() << "\n";
    }
    
    return 0;
}
