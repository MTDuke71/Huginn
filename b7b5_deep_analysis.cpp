#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

using namespace BitboardMoveGen;

// Convert function
SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.is_capture = move.is_capture;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    simple_move.is_promotion = move.is_promotion;
    simple_move.promotion_type = move.promotion_type;
    return simple_move;
}

// Move to string function
std::string move_to_string(const SimpleBitboardMove& move) {
    std::string result;
    result += char('a' + (move.from_64 % 8));
    result += char('1' + (move.from_64 / 8));
    result += char('a' + (move.to_64 % 8));
    result += char('1' + (move.to_64 / 8));
    return result;
}

// Square to algebraic for en passant display
std::string square_to_alg(int square) {
    if (square == -1) return "-";
    return std::string(1, 'a' + (square % 8)) + std::string(1, '1' + (square / 8));
}

int main() {
    std::cout << "=== Deep Dive: b7b5 Move Analysis ===\n\n";
    
    // Start from the initial position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Playing sequence: a2a4 a7a6 a4a5 b7b5\n\n";
    
    // Make a2a4 move
    SimpleBitboardMove a2a4;
    a2a4.from_64 = 8;  // a2
    a2a4.to_64 = 24;   // a4
    a2a4.is_capture = false;
    a2a4.is_ep_capture = false;
    a2a4.is_castling = false;
    a2a4.is_promotion = false;
    a2a4.promotion_type = PieceType::None;
    
    pos.make_move_with_undo(a2a4);
    std::cout << "After a2a4: ep_square = " << square_to_alg(pos.ep_square_64) << "\n";
    
    // Make a7a6 move
    SimpleBitboardMove a7a6;
    a7a6.from_64 = 48;  // a7
    a7a6.to_64 = 40;    // a6
    a7a6.is_capture = false;
    a7a6.is_ep_capture = false;
    a7a6.is_castling = false;
    a7a6.is_promotion = false;
    a7a6.promotion_type = PieceType::None;
    
    pos.make_move_with_undo(a7a6);
    std::cout << "After a7a6: ep_square = " << square_to_alg(pos.ep_square_64) << "\n";
    
    // Make a4a5 move
    SimpleBitboardMove a4a5;
    a4a5.from_64 = 24;  // a4
    a4a5.to_64 = 32;    // a5
    a4a5.is_capture = false;
    a4a5.is_ep_capture = false;
    a4a5.is_castling = false;
    a4a5.is_promotion = false;
    a4a5.promotion_type = PieceType::None;
    
    pos.make_move_with_undo(a4a5);
    std::cout << "After a4a5: ep_square = " << square_to_alg(pos.ep_square_64) << "\n";
    
    // Make b7b5 move (the problematic move)
    SimpleBitboardMove b7b5;
    b7b5.from_64 = 49;  // b7
    b7b5.to_64 = 33;    // b5
    b7b5.is_capture = false;
    b7b5.is_ep_capture = false;
    b7b5.is_castling = false;
    b7b5.is_promotion = false;
    b7b5.promotion_type = PieceType::None;
    
    pos.make_move_with_undo(b7b5);
    std::cout << "After b7b5: ep_square = " << square_to_alg(pos.ep_square_64) << "\n\n";
    
    std::cout << "Position after a2a4 a7a6 a4a5 b7b5:\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    std::cout << "White pawn on a5: " << (pos.piece_at(32) != Piece::None ? "✅" : "❌") << "\n";
    std::cout << "Black pawn on a6: " << (pos.piece_at(40) != Piece::None ? "✅" : "❌") << "\n";
    std::cout << "Black pawn on b5: " << (pos.piece_at(33) != Piece::None ? "✅" : "❌") << "\n";
    std::cout << "En passant square: " << square_to_alg(pos.ep_square_64) << "\n\n";
    
    std::cout << "=== Critical Analysis ===\n";
    std::cout << "The current position has:\n";
    std::cout << "- White pawn on a5\n";
    std::cout << "- Black pawn on a6 \n";
    std::cout << "- Black pawn on b5 (just moved 2 squares)\n";
    std::cout << "- En passant square at b6 (should allow axb6 en passant capture)\n\n";
    
    if (pos.ep_square_64 == 41) {  // b6 square
        std::cout << "✅ En passant square correctly set to b6\n";
        std::cout << "White should be able to play axb6 (en passant capture)\n\n";
        
        // Check if axb6 en passant capture is available
        BitboardMoveList moves;
        generate_all_moves(pos, moves);
        
        bool found_en_passant = false;
        std::cout << "White's legal moves:\n";
        
        for (const auto& move : moves.moves) {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos.is_legal_move(simple_move)) {
                std::string move_str = move_to_string(simple_move);
                
                if (simple_move.is_ep_capture) {
                    std::cout << "✅ " << move_str << " (EN PASSANT CAPTURE!)\n";
                    found_en_passant = true;
                } else {
                    std::cout << "   " << move_str << "\n";
                }
            }
        }
        
        if (!found_en_passant) {
            std::cout << "\n❌ ERROR: En passant capture axb6 is missing!\n";
            std::cout << "This is likely the source of our -1 node error.\n";
        } else {
            std::cout << "\n✅ En passant capture is properly generated.\n";
            std::cout << "The bug must be elsewhere in this line.\n";
        }
        
    } else {
        std::cout << "❌ En passant square incorrectly set\n";
        std::cout << "Expected: b6 (square 41), Got: " << pos.ep_square_64 << "\n";
    }
    
    return 0;
}