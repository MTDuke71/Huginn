#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <map>

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

int main() {
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << "Checking for duplicate moves..." << std::endl;
    
    std::map<std::string, int> move_counts;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = move_to_string(simple_move);
            move_counts[move_str]++;
            
            if (move_counts[move_str] > 1) {
                std::cout << "DUPLICATE FOUND: " << move_str << " appears " << move_counts[move_str] << " times" << std::endl;
            }
        }
    }
    
    std::cout << "Total unique legal moves: " << move_counts.size() << std::endl;
    
    // Show all moves and their counts
    for (const auto& entry : move_counts) {
        if (entry.second > 1) {
            std::cout << entry.first << ": " << entry.second << " times" << std::endl;
        }
    }
    
    return 0;
}