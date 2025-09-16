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

int main() {
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing specific moves that give 0..." << std::endl;
    
    // Test a2a3 first (this should work)
    SimpleBitboardMove a2a3;
    a2a3.from_64 = 8;   // a2
    a2a3.to_64 = 16;    // a3
    a2a3.is_capture = false;
    a2a3.is_ep_capture = false;
    a2a3.is_castling = false;
    a2a3.is_promotion = false;
    a2a3.promotion_type = PieceType::None;
    
    std::cout << "Is a2a3 legal in starting position? " << pos.is_legal_move(a2a3) << std::endl;
    
    // Make a2a3 and see what black moves are legal
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(a2a3);
    std::cout << "After a2a3, side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    
    // Test c7c6 (which is giving 0 in perft)
    SimpleBitboardMove c7c6;
    c7c6.from_64 = 50;  // c7
    c7c6.to_64 = 42;    // c6
    c7c6.is_capture = false;
    c7c6.is_ep_capture = false;
    c7c6.is_castling = false;
    c7c6.is_promotion = false;
    c7c6.promotion_type = PieceType::None;
    
    std::cout << "Is c7c6 legal after a2a3? " << pos.is_legal_move(c7c6) << std::endl;
    
    // Generate all moves and see which ones are legal
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    int legal_count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            legal_count++;
            char from_file = 'a' + file_of_64(move.from_64);
            char from_rank = '1' + rank_of_64(move.from_64);
            char to_file = 'a' + file_of_64(move.to_64);
            char to_rank = '1' + rank_of_64(move.to_64);
            std::cout << from_file << from_rank << to_file << to_rank << " ";
        }
    }
    std::cout << "\nTotal legal moves after a2a3: " << legal_count << std::endl;
    
    pos.unmake_move(a2a3, undo);
    
    return 0;
}