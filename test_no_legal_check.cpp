#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

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

// Divide function WITHOUT legal checking
void divide_no_legal_check(BitboardPosition& pos, int depth) {
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::vector<std::pair<std::string, uint64_t>> results;
    uint64_t total_nodes = 0;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        // Skip legal checking to avoid double make/unmake
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
        uint64_t nodes = (depth <= 1) ? 1 : perft_no_legal_check(pos, depth - 1);
        pos.unmake_move(simple_move, undo_info);
        
        std::string move_str = move_to_string(simple_move);
        results.push_back({move_str, nodes});
        total_nodes += nodes;
    }
    
    // Sort results alphabetically for easier comparison
    std::sort(results.begin(), results.end());
    
    std::cout << "Divide " << depth << " from starting position (NO LEGAL CHECK):" << std::endl;
    std::cout << "==========================================================" << std::endl;
    
    for (const auto& result : results) {
        std::cout << std::setw(6) << result.first << ": " << std::setw(6) << result.second << std::endl;
    }
    
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Total: " << total_nodes << std::endl;
}

int main() {
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing Divide WITHOUT is_legal_move() calls" << std::endl;
    std::cout << "=============================================" << std::endl;
    divide_no_legal_check(pos, 3);
    
    return 0;
}