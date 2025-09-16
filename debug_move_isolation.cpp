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

// Simple perft count function for depth 1 only
uint64_t count_legal_moves(BitboardPosition& pos) {
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            count++;
        }
    }
    
    return count;
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
    
    std::cout << "Testing moves that give 0 nodes in the divide function:" << std::endl;
    std::cout << "=======================================================" << std::endl;
    
    // Test individual moves by simulating the divide loop but stopping at the problematic moves
    std::vector<std::string> problem_moves = {"h2h3", "c2c4", "g2g4", "b1c3"};
    
    for (const std::string& move_str : problem_moves) {
        std::cout << "\nTesting " << move_str << ":" << std::endl;
        
        // Find this move in the move list
        BitboardMoveList moves;
        generate_all_moves(pos, moves);
        
        for (const auto& move : moves.moves) {
            SimpleBitboardMove simple_move = convert_move(move);
            std::string current_move_str = move_to_string(simple_move);
            
            if (current_move_str == move_str) {
                std::cout << "  Found move " << move_str << std::endl;
                
                if (pos.is_legal_move(simple_move)) {
                    std::cout << "  Move is legal" << std::endl;
                    
                    // Make the move
                    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
                    
                    // Count legal moves in the resulting position
                    uint64_t legal_moves_after = count_legal_moves(pos);
                    std::cout << "  Legal moves after " << move_str << ": " << legal_moves_after << std::endl;
                    
                    // Try perft(1) to see if it matches
                    uint64_t perft1_result = perft_legal(pos, 1);
                    std::cout << "  Perft(1) after " << move_str << ": " << perft1_result << std::endl;
                    
                    // Try perft(2) which should be the result we want
                    uint64_t perft2_result = perft_legal(pos, 2);
                    std::cout << "  Perft(2) after " << move_str << ": " << perft2_result << std::endl;
                    
                    // Unmake the move
                    pos.unmake_move(simple_move, undo_info);
                    std::cout << "  Move unmade" << std::endl;
                    
                } else {
                    std::cout << "  ERROR: Move is not legal!" << std::endl;
                }
                break;
            }
        }
    }
    
    return 0;
}