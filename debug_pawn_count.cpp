#include "position.hpp"
#include "chess_types.hpp"
#include <iostream>

int main() {
    Position pos;
    pos.set_startpos();
    
    std::cout << "Initial position pawn count: " << pos.piece_counts[size_t(PieceType::Pawn)] << std::endl;
    
    // Add a black pawn on e4
    pos.set(sq(File::E, Rank::R4), make_piece(Color::Black, PieceType::Pawn));
    pos.rebuild_counts();
    std::cout << "After adding black pawn on e4: " << pos.piece_counts[size_t(PieceType::Pawn)] << std::endl;
    
    // Make capture move d2xe4
    S_MOVE move = make_capture(sq(File::D, Rank::R2), sq(File::E, Rank::R4), PieceType::Pawn);
    pos.make_move_with_undo(move);
    std::cout << "After capture d2xe4: " << pos.piece_counts[size_t(PieceType::Pawn)] << std::endl;
    
    return 0;
}
