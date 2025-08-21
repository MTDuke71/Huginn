#include <iostream>
#include "position.hpp"
#include "init.hpp"
#include "board120.hpp"
#include "chess_types.hpp"

int main() {
    Huginn::init();
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "=== Material Tracking Debug ===" << std::endl;
    std::cout << "Initial material: White=" << pos.get_material_score(Color::White) 
              << ", Black=" << pos.get_material_score(Color::Black)
              << ", Balance=" << pos.get_material_balance() << std::endl;
    
    // Place an extra black pawn on e4
    std::cout << "\nPlacing black pawn on e4..." << std::endl;
    pos.set(sq(File::E, Rank::R4), make_piece(Color::Black, PieceType::Pawn));
    pos.rebuild_counts();
    
    std::cout << "After placing pawn: White=" << pos.get_material_score(Color::White) 
              << ", Black=" << pos.get_material_score(Color::Black)
              << ", Balance=" << pos.get_material_balance() << std::endl;
    
    // Make a capture move: d2 pawn captures e4 pawn
    std::cout << "\nMaking capture move d2xE4..." << std::endl;
    Move move;
    move.from = sq(File::D, Rank::R2);  // d2
    move.to = sq(File::E, Rank::R4);    // e4 (capture black pawn)
    move.promo = PieceType::None;
    
    // Check what piece is being captured
    Piece captured = pos.at(move.to);
    std::cout << "Piece being captured: " << int(captured) << " (should be black pawn)" << std::endl;
    std::cout << "Value of captured piece: " << value_of(captured) << std::endl;
    
    pos.make_move_with_undo(move);
    
    std::cout << "After capture: White=" << pos.get_material_score(Color::White) 
              << ", Black=" << pos.get_material_score(Color::Black)
              << ", Balance=" << pos.get_material_balance() << std::endl;
    
    return 0;
}
