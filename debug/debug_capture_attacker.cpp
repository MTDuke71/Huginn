#include <iostream>
#include <vector>
#include <string>

#include "chess_types.hpp"
#include "movegen_enhanced.hpp"

int main() {
    // Test the CaptureAttacker position: "8/8/8/8/4K3/8/3N4/3q4 w - - 0 1"
    Position pos;
    pos.set_from_fen("8/8/8/8/4K3/8/3N4/3q4 w - - 0 1");
    
    std::cout << "Position: King on E4, Knight on D2, Queen on D1" << std::endl;
    std::cout << "King square: " << pos.king_sq[int(Color::White)] << " (should be " << sq(File::E, Rank::R4) << ")" << std::endl;
    
    // Check if king is in check
    bool king_in_check = SqAttacked(pos.king_sq[int(Color::White)], pos, Color::Black);
    std::cout << "King in check: " << (king_in_check ? "YES" : "NO") << std::endl;
    
    // Generate moves
    MoveList pseudo_moves, legal_moves;
    generate_pseudo_legal_moves(pos, pseudo_moves);
    generate_legal_moves(pos, legal_moves);
    
    std::cout << "Pseudo-legal moves: " << pseudo_moves.size() << std::endl;
    std::cout << "Legal moves: " << legal_moves.size() << std::endl;
    
    // Check knight moves specifically
    int knight_square = sq(File::D, Rank::R2);
    int queen_square = sq(File::D, Rank::R1);
    
    std::cout << "Knight square: " << knight_square << std::endl;
    std::cout << "Queen square: " << queen_square << std::endl;
    
    // Look for the knight capture move
    bool found_knight_capture = false;
    for (size_t i = 0; i < pseudo_moves.size(); ++i) {
        const auto& move = pseudo_moves[i];
        if (move.get_from() == knight_square && move.get_to() == queen_square) {
            found_knight_capture = true;
            std::cout << "Knight capture move found in pseudo-legal moves" << std::endl;
            
            // Test if this move is legal
            bool is_legal = is_legal_move(pos, move);
            std::cout << "Knight capture move is legal: " << (is_legal ? "YES" : "NO") << std::endl;
            
            // Simulate the move to see what happens
            Position temp_pos = pos;
            temp_pos.board[knight_square] = Piece::None;
            temp_pos.board[queen_square] = make_piece(Color::White, PieceType::Knight);
            
            bool king_still_in_check = SqAttacked(temp_pos.king_sq[int(Color::White)], temp_pos, Color::Black);
            std::cout << "After knight captures queen, king still in check: " << (king_still_in_check ? "YES" : "NO") << std::endl;
            break;
        }
    }
    
    if (!found_knight_capture) {
        std::cout << "Knight capture move NOT found in pseudo-legal moves!" << std::endl;
    }
    
    return 0;
}
