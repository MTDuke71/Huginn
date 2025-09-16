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
    std::cout << "Testing sequence of moves leading up to h2h3 corruption" << std::endl;
    std::cout << "=======================================================" << std::endl;
    
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Generate all moves to get the exact sequence
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << "Processing moves in sequence (simulating divide function):" << std::endl;
    
    int move_count = 0;
    for (const auto& move : moves.moves) {
        move_count++;
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        std::cout << "\nMove " << move_count << ": " << move_str << std::endl;
        
        if (pos.is_legal_move(simple_move)) {
            std::cout << "  Legal: Yes" << std::endl;
            
            // Simulate the exact divide function logic
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            uint64_t nodes = (3 <= 1) ? 1 : perft_legal(pos, 2);
            pos.unmake_move(simple_move, undo_info);
            
            std::cout << "  Nodes: " << nodes << std::endl;
            
            // If this is h2h3 or one of the problem moves, break here to see the state
            if (move_str == "h2h3") {
                std::cout << "  *** This is h2h3 - position state after processing " << move_count << " moves ***" << std::endl;
                
                // Test h2h3 again to see if position is corrupted
                std::cout << "  Testing h2h3 again after " << (move_count-1) << " previous moves:" << std::endl;
                
                if (pos.is_legal_move(simple_move)) {
                    BitboardPosition::UndoInfo undo_info2 = pos.make_move_with_undo(simple_move);
                    uint64_t nodes2 = perft_legal(pos, 2);
                    pos.unmake_move(simple_move, undo_info2);
                    std::cout << "    h2h3 perft(2): " << nodes2 << std::endl;
                } else {
                    std::cout << "    h2h3 is no longer legal!" << std::endl;
                }
                break;
            }
            
        } else {
            std::cout << "  Legal: No" << std::endl;
        }
        
        // Stop at move 10 to focus on the area around h2h3 (which is move 8)
        if (move_count >= 10) break;
    }
    
    return 0;
}