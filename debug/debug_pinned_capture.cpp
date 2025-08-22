#include <iostream>
#include "chess_types.hpp" 
#include "movegen_enhanced.hpp"

int main() {
    // Test the pinned piece capture scenario
    Position pos;
    pos.set_from_fen("8/8/8/3K4/8/8/3R4/3r4 w - - 0 1");
    
    std::cout << "Position: King on D5, White Rook on D2, Black Rook on D1" << std::endl;
    
    // Check if white king is in check initially
    bool king_in_check = SqAttacked(pos.king_sq[int(Color::White)], pos, Color::Black);
    std::cout << "King in check initially: " << (king_in_check ? "YES" : "NO") << std::endl;
    
    // Test the specific move D2 -> D1 (rook captures rook)
    int from_sq = sq(File::D, Rank::R2);
    int to_sq = sq(File::D, Rank::R1);
    
    std::cout << "Testing move from D2(" << from_sq << ") to D1(" << to_sq << ")" << std::endl;
    
    // Create the move
    S_MOVE test_move(from_sq, to_sq, PieceType::Rook); // Capturing the black rook
    
    // Test if it's legal
    bool is_legal = is_legal_move(pos, test_move);
    std::cout << "Move D2->D1 is legal: " << (is_legal ? "YES" : "NO") << std::endl;
    
    // Let's manually simulate what happens
    Position temp_pos = pos;
    temp_pos.board[from_sq] = Piece::None;           // Remove white rook from D2
    temp_pos.board[to_sq] = make_piece(Color::White, PieceType::Rook); // Place white rook on D1
    
    std::cout << "\nAfter simulating the move:" << std::endl;
    std::cout << "White rook on D1: " << (temp_pos.board[to_sq] == make_piece(Color::White, PieceType::Rook) ? "YES" : "NO") << std::endl;
    std::cout << "D2 is empty: " << (temp_pos.board[from_sq] == Piece::None ? "YES" : "NO") << std::endl;
    
    // Check if king would be in check after this move
    bool king_in_check_after = SqAttacked(temp_pos.king_sq[int(Color::White)], temp_pos, Color::Black);
    std::cout << "King in check after move: " << (king_in_check_after ? "YES" : "NO") << std::endl;
    
    // Print all pieces on the board after the move
    std::cout << "\nPieces on board after move:" << std::endl;
    for (int sq = 21; sq <= 98; ++sq) {
        if (SQOFFBOARD(sq)) continue;
        if (temp_pos.board[sq] != Piece::None) {
            std::cout << "Square " << sq << ": " << static_cast<int>(temp_pos.board[sq]) << std::endl;
        }
    }
    
    return 0;
}
