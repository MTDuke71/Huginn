#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

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

// Test what happens after making moves that give 0 in perft
int main() {
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing what happens after moves that give 0..." << std::endl;
    
    // Test b1c3 (gives 0, should be 440)
    SimpleBitboardMove b1c3;
    b1c3.from_64 = 1;   // b1
    b1c3.to_64 = 18;    // c3  
    b1c3.is_capture = false;
    b1c3.is_ep_capture = false;
    b1c3.is_castling = false;
    b1c3.is_promotion = false;
    b1c3.promotion_type = PieceType::None;
    
    std::cout << "\nAfter b1c3:" << std::endl;
    BitboardPosition::UndoInfo undo1 = pos.make_move_with_undo(b1c3);
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    
    // Count legal moves for black
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    int legal_count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            legal_count++;
        }
    }
    std::cout << "Legal moves for black: " << legal_count << std::endl;
    
    pos.unmake_move(b1c3, undo1);
    
    // Test c2c4 (gives 0, should be 441)
    SimpleBitboardMove c2c4;
    c2c4.from_64 = 10;  // c2
    c2c4.to_64 = 26;    // c4
    c2c4.is_capture = false;
    c2c4.is_ep_capture = false;
    c2c4.is_castling = false;
    c2c4.is_promotion = false;
    c2c4.promotion_type = PieceType::None;
    
    std::cout << "\nAfter c2c4:" << std::endl;
    BitboardPosition::UndoInfo undo2 = pos.make_move_with_undo(c2c4);
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    
    // Count legal moves for black
    moves.moves.clear();
    generate_all_moves(pos, moves);
    
    legal_count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            legal_count++;
        }
    }
    std::cout << "Legal moves for black: " << legal_count << std::endl;
    
    pos.unmake_move(c2c4, undo2);
    
    // For comparison, test a move that works (a2a3)
    SimpleBitboardMove a2a3;
    a2a3.from_64 = 8;   // a2
    a2a3.to_64 = 16;    // a3
    a2a3.is_capture = false;
    a2a3.is_ep_capture = false;
    a2a3.is_castling = false;
    a2a3.is_promotion = false;
    a2a3.promotion_type = PieceType::None;
    
    std::cout << "\nAfter a2a3 (for comparison):" << std::endl;
    BitboardPosition::UndoInfo undo3 = pos.make_move_with_undo(a2a3);
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    
    // Count legal moves for black
    moves.moves.clear();
    generate_all_moves(pos, moves);
    
    legal_count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            legal_count++;
        }
    }
    std::cout << "Legal moves for black: " << legal_count << std::endl;
    
    pos.unmake_move(a2a3, undo3);
    
    return 0;
}