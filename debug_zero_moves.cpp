#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;

// Test specific moves that are giving 0 in perft
int main() {
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing moves that give 0 in perft divide..." << std::endl;
    
    // Test b1c3 (gives 0, should be 440)
    SimpleBitboardMove b1c3;
    b1c3.from_64 = 1;   // b1
    b1c3.to_64 = 18;    // c3  
    b1c3.is_capture = false;
    b1c3.is_ep_capture = false;
    b1c3.is_castling = false;
    b1c3.is_promotion = false;
    b1c3.promotion_type = PieceType::None;
    
    std::cout << "b1c3 legal? " << pos.is_legal_move(b1c3) << std::endl;
    
    if (pos.is_legal_move(b1c3)) {
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(b1c3);
        std::cout << "After b1c3, white king square: " << pos.king_square_64[0] << std::endl;
        std::cout << "After b1c3, is white king in check? " << pos.is_square_attacked(pos.king_square_64[0], Color::Black) << std::endl;
        pos.unmake_move(b1c3, undo);
    }
    
    // Test c2c4 (gives 0, should be 441)
    SimpleBitboardMove c2c4;
    c2c4.from_64 = 10;  // c2
    c2c4.to_64 = 26;    // c4
    c2c4.is_capture = false;
    c2c4.is_ep_capture = false;
    c2c4.is_castling = false;
    c2c4.is_promotion = false;
    c2c4.promotion_type = PieceType::None;
    
    std::cout << "c2c4 legal? " << pos.is_legal_move(c2c4) << std::endl;
    
    // Test g2g4 (gives 0, should be 421)
    SimpleBitboardMove g2g4;
    g2g4.from_64 = 14;  // g2
    g2g4.to_64 = 30;    // g4
    g2g4.is_capture = false;
    g2g4.is_ep_capture = false;
    g2g4.is_castling = false;
    g2g4.is_promotion = false;
    g2g4.promotion_type = PieceType::None;
    
    std::cout << "g2g4 legal? " << pos.is_legal_move(g2g4) << std::endl;
    
    // Test h2h3 (gives 0, should be 420)
    SimpleBitboardMove h2h3;
    h2h3.from_64 = 15;  // h2
    h2h3.to_64 = 23;    // h3
    h2h3.is_capture = false;
    h2h3.is_ep_capture = false;
    h2h3.is_castling = false;
    h2h3.is_promotion = false;
    h2h3.promotion_type = PieceType::None;
    
    std::cout << "h2h3 legal? " << pos.is_legal_move(h2h3) << std::endl;
    
    // Let's also test a move that gives a wrong high count
    // Test f2f3 (gives 2817, should be 380)
    SimpleBitboardMove f2f3;
    f2f3.from_64 = 13;  // f2
    f2f3.to_64 = 21;    // f3
    f2f3.is_capture = false;
    f2f3.is_ep_capture = false;
    f2f3.is_castling = false;
    f2f3.is_promotion = false;
    f2f3.promotion_type = PieceType::None;
    
    std::cout << "f2f3 legal? " << pos.is_legal_move(f2f3) << std::endl;
    
    if (pos.is_legal_move(f2f3)) {
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(f2f3);
        std::cout << "After f2f3, white king square: " << pos.king_square_64[0] << std::endl;
        std::cout << "After f2f3, is white king in check? " << pos.is_square_attacked(pos.king_square_64[0], Color::Black) << std::endl;
        pos.unmake_move(f2f3, undo);
    }
    
    return 0;
}