#include <iostream>
#include <vector>
#include "chess_types.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"

int main() {
    // Minimal test - just check the specific test case
    std::cout << "Testing king capture legal move validation..." << std::endl;
    
    // Position: 8/8/8/3r4/4K3/8/8/8 w - - 0 1
    // King on E4 (square 55), Black rook on D5 (square 64)
    Position pos;
    
    // Clear the board
    for (int i = 0; i < 120; i++) {
        pos.board[i] = Piece::None;
    }
    
    // Set the pieces
    pos.board[55] = Piece::WhiteKing;  // E4
    pos.board[64] = Piece::BlackRook;  // D5
    pos.side_to_move = Color::White;
    pos.king_sq[0] = 55;  // White king
    pos.king_sq[1] = -1;  // No black king for this test
    
    // Test the move from E4 to D5 (king captures rook)
    S_MOVE move(55, 64);  // from E4 to D5
    
    std::cout << "Testing move from square 55 (E4) to square 64 (D5)" << std::endl;
    std::cout << "Piece at 55: " << int(pos.board[55]) << std::endl;
    std::cout << "Piece at 64: " << int(pos.board[64]) << std::endl;
    
    // Test if square 64 is attacked by black (the rook)
    bool attacked_before = SqAttacked(64, pos, Color::Black);
    std::cout << "Square 64 attacked by Black before move: " << attacked_before << std::endl;
    
    // Remove the king temporarily and check if square is still attacked
    Position temp_pos = pos;
    temp_pos.board[55] = Piece::None;
    bool attacked_after_king_removed = SqAttacked(64, pos, Color::Black);
    std::cout << "Square 64 attacked by Black after removing king: " << attacked_after_king_removed << std::endl;
    
    // Test the legal move function
    bool is_legal = is_legal_move(pos, move);
    std::cout << "Is the move legal according to is_legal_move: " << is_legal << std::endl;
    std::cout << "Expected: false (king cannot capture piece that attacks the destination square)" << std::endl;
    
    return 0;
}
