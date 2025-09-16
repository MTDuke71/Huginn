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
    std::cout << "Testing with fresh position for each move:" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    // Test each of the problem moves with a completely fresh position
    std::vector<std::string> problem_moves = {"h2h3", "c2c4", "g2g4", "b1c3"};
    
    for (const std::string& target_move : problem_moves) {
        // Create a completely fresh position for each test
        BitboardPosition pos;
        pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        
        std::cout << "\nTesting " << target_move << " with fresh position:" << std::endl;
        
        // Generate moves
        BitboardMoveList moves;
        generate_all_moves(pos, moves);
        
        // Find the target move
        for (const auto& move : moves.moves) {
            SimpleBitboardMove simple_move = convert_move(move);
            std::string move_str = move_to_string(simple_move);
            
            if (move_str == target_move) {
                if (pos.is_legal_move(simple_move)) {
                    // Apply the exact same logic as the divide function
                    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
                    uint64_t nodes = (3 <= 1) ? 1 : perft_legal(pos, 2);
                    pos.unmake_move(simple_move, undo_info);
                    
                    std::cout << "  " << target_move << " -> " << nodes << " nodes" << std::endl;
                } else {
                    std::cout << "  " << target_move << " is not legal!" << std::endl;
                }
                break;
            }
        }
    }
    
    std::cout << "\n\nNow testing the same moves in sequence (like divide):" << std::endl;
    std::cout << "=====================================================" << std::endl;
    
    // Create one position and test the moves in sequence
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Generate all moves once
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    int move_count = 0;
    for (const auto& move : moves.moves) {
        move_count++;
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            uint64_t nodes = (3 <= 1) ? 1 : perft_legal(pos, 2);
            pos.unmake_move(simple_move, undo_info);
            
            // Only show the problem moves
            if (move_str == "h2h3" || move_str == "c2c4" || move_str == "g2g4" || move_str == "b1c3") {
                std::cout << "Move " << move_count << ": " << move_str << " -> " << nodes << " nodes" << std::endl;
            }
        }
        
        if (move_count >= 20) break;
    }
    
    return 0;
}