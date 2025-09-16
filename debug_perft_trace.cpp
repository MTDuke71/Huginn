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

// Perft WITH debugging
uint64_t perft_debug(BitboardPosition& pos, int depth, const std::string& prefix = "") {
    if (depth == 0) {
        std::cout << prefix << "depth 0 -> return 1" << std::endl;
        return 1;
    }
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << prefix << "depth " << depth << ", generated " << moves.moves.size() << " moves" << std::endl;
    
    uint64_t nodes = 0;
    int legal_count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        if (pos.is_legal_move(simple_move)) {
            legal_count++;
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            
            char from_file = 'a' + file_of_64(move.from_64);
            char from_rank = '1' + rank_of_64(move.from_64);
            char to_file = 'a' + file_of_64(move.to_64);
            char to_rank = '1' + rank_of_64(move.to_64);
            std::string move_str = std::string(1, from_file) + from_rank + to_file + to_rank;
            
            std::cout << prefix << "Legal move " << legal_count << ": " << move_str << std::endl;
            
            uint64_t subnodes = perft_debug(pos, depth - 1, prefix + "  ");
            nodes += subnodes;
            
            std::cout << prefix << "Move " << move_str << " contributed " << subnodes << " nodes" << std::endl;
            
            pos.unmake_move(simple_move, undo_info);
        }
    }
    
    std::cout << prefix << "Total legal moves: " << legal_count << ", total nodes: " << nodes << std::endl;
    return nodes;
}

int main() {
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Test b1c3 specifically (this gives 0 in perft divide)
    SimpleBitboardMove b1c3;
    b1c3.from_64 = 1;   // b1
    b1c3.to_64 = 18;    // c3  
    b1c3.is_capture = false;
    b1c3.is_ep_capture = false;
    b1c3.is_castling = false;
    b1c3.is_promotion = false;
    b1c3.promotion_type = PieceType::None;
    
    std::cout << "Debugging perft after b1c3..." << std::endl;
    std::cout << "================================" << std::endl;
    
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(b1c3);
    
    std::cout << "Position after b1c3, side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    
    uint64_t result = perft_debug(pos, 2);
    
    std::cout << "Final result for depth 2 after b1c3: " << result << std::endl;
    
    pos.unmake_move(b1c3, undo);
    
    return 0;
}