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
    result += char('a' + file_of_64(move.from_64));
    result += char('1' + rank_of_64(move.from_64));
    result += char('a' + file_of_64(move.to_64));
    result += char('1' + rank_of_64(move.to_64));
    return result;
}

// Perft WITHOUT legal checking (assume all generated moves are pseudo-legal)
uint64_t perft_no_legal_check(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        // Skip legal checking to avoid double make/unmake
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
        nodes += perft_no_legal_check(pos, depth - 1);
        pos.unmake_move(simple_move, undo_info);
    }
    
    return nodes;
}

int main() {
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing divide WITHOUT is_legal_move() to avoid double make/unmake:" << std::endl;
    std::cout << "=================================================================" << std::endl;
    
    // Generate all moves
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << "Generated " << moves.moves.size() << " moves" << std::endl;
    
    int move_count = 0;
    for (const auto& move : moves.moves) {
        move_count++;
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        std::cout << "Move " << move_count << ": " << move_str;
        
        // Skip legal checking completely
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
        uint64_t nodes = (3 <= 1) ? 1 : perft_no_legal_check(pos, 2);
        pos.unmake_move(simple_move, undo_info);
        
        std::cout << " -> " << nodes << " nodes";
        
        // Check if this is one of the problem moves
        if (move_str == "h2h3" || move_str == "c2c4" || move_str == "g2g4" || move_str == "b1c3") {
            std::cout << " *** FORMER PROBLEM MOVE ***";
        }
        
        std::cout << std::endl;
        
        // Stop after the 20 moves to match the original debug output
        if (move_count >= 20) break;
    }
    
    return 0;
}