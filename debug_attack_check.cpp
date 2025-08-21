#include <iostream>
#include "src/chess_types.hpp"
#include "src/movegen.hpp"

int main() {
    // Test if queen on D1 attacks king on E4 in position "8/8/8/8/4K3/8/3N4/3q4 w - - 0 1"
    Position pos;
    pos.set_from_fen("8/8/8/8/4K3/8/3N4/3q4 w - - 0 1");
    
    int king_square = sq(File::E, Rank::R4);
    int queen_square = sq(File::D, Rank::R1);
    
    std::cout << "King square (E4): " << king_square << std::endl;
    std::cout << "Queen square (D1): " << queen_square << std::endl;
    
    // Check if queen attacks king
    bool queen_attacks_king = SqAttacked(king_square, pos, Color::Black);
    std::cout << "Queen attacks king: " << (queen_attacks_king ? "YES" : "NO") << std::endl;
    
    // Let's also check manually by looking at the queen's attacks
    std::cout << "\nQueen on D1 can attack these squares:" << std::endl;
    for (int sq = 21; sq <= 98; ++sq) {
        if (SQOFFBOARD(sq)) continue;
        
        Position test_pos = pos;
        test_pos.board[sq] = make_piece(Color::White, PieceType::King); // Place a test white king
        
        if (SqAttacked(sq, test_pos, Color::Black)) {
            std::cout << "Square " << sq;
            if (sq == king_square) std::cout << " (E4 - KING SQUARE!)";
            std::cout << std::endl;
        }
    }
    
    return 0;
}
