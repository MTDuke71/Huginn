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

// Perft WITH legal checking
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            nodes += perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo_info);
        }
    }
    
    return nodes;
}

int main() {
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Test EXACTLY what the divide function does for b1c3
    SimpleBitboardMove b1c3;
    b1c3.from_64 = 1;   // b1
    b1c3.to_64 = 18;    // c3  
    b1c3.is_capture = false;
    b1c3.is_ep_capture = false;
    b1c3.is_castling = false;
    b1c3.is_promotion = false;
    b1c3.promotion_type = PieceType::None;
    
    std::cout << "Testing exact divide logic for b1c3 at depth 3..." << std::endl;
    
    if (pos.is_legal_move(b1c3)) {
        std::cout << "b1c3 is legal" << std::endl;
        
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(b1c3);
        
        int depth = 3;
        uint64_t nodes = (depth <= 1) ? 1 : perft_legal(pos, depth - 1);
        
        std::cout << "depth = " << depth << std::endl;
        std::cout << "depth <= 1? " << (depth <= 1 ? "true" : "false") << std::endl;
        std::cout << "Calling perft_legal with depth " << (depth - 1) << std::endl;
        std::cout << "Result: " << nodes << std::endl;
        
        pos.unmake_move(b1c3, undo_info);
    } else {
        std::cout << "b1c3 is NOT legal" << std::endl;
    }
    
    return 0;
}