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
    
    std::cout << "Debug divide function for moves that give 0..." << std::endl;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << "Total moves generated: " << moves.moves.size() << std::endl;
    
    // Look specifically for b1c3, c2c4, g2g4, h2h3
    std::vector<std::string> problem_moves = {"b1c3", "c2c4", "g2g4", "h2h3"};
    
    for (const auto& target_move : problem_moves) {
        std::cout << "\nLooking for move: " << target_move << std::endl;
        bool found = false;
        
        for (const auto& move : moves.moves) {
            SimpleBitboardMove simple_move = convert_move(move);
            std::string move_str = move_to_string(simple_move);
            
            if (move_str == target_move) {
                found = true;
                std::cout << "Found " << move_str << "!" << std::endl;
                std::cout << "  from_64: " << move.from_64 << std::endl;
                std::cout << "  to_64: " << move.to_64 << std::endl;
                std::cout << "  is_legal: " << pos.is_legal_move(simple_move) << std::endl;
                
                if (pos.is_legal_move(simple_move)) {
                    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
                    uint64_t nodes = perft_legal(pos, 2);  // depth 3 - 1 = 2
                    pos.unmake_move(simple_move, undo_info);
                    std::cout << "  perft(2) result: " << nodes << std::endl;
                } else {
                    std::cout << "  Move is ILLEGAL!" << std::endl;
                }
                break;
            }
        }
        
        if (!found) {
            std::cout << "Move " << target_move << " NOT FOUND in generated moves!" << std::endl;
        }
    }
    
    return 0;
}