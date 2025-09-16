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

// Perft WITH detailed breakdown for debugging
uint64_t perft_divide_detailed(BitboardPosition& pos, int depth, const std::string& indent = "") {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t total_nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            uint64_t nodes = perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo_info);
            
            std::cout << indent << move_to_string(simple_move) << ": " << nodes << std::endl;
            total_nodes += nodes;
        }
    }
    
    return total_nodes;
}

int main() {
    std::cout << "Testing a2a4 from starting position with perft(2)" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Create a2a4 move
    SimpleBitboardMove a2a4;
    a2a4.from_64 = 8;  // a2
    a2a4.to_64 = 24;   // a4
    a2a4.is_capture = false;
    a2a4.is_ep_capture = false;
    a2a4.is_castling = false;
    a2a4.is_promotion = false;
    a2a4.promotion_type = PieceType::None;
    
    std::cout << "Testing a2a4 legality..." << std::endl;
    bool is_legal = pos.is_legal_move(a2a4);
    std::cout << "a2a4 is legal: " << (is_legal ? "YES" : "NO") << std::endl;
    
    if (!is_legal) {
        std::cout << "ERROR: a2a4 should be legal!" << std::endl;
        return 1;
    }
    
    std::cout << "\nMaking a2a4 move and getting position after..." << std::endl;
    
    // Make the a2a4 move
    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(a2a4);
    
    std::cout << "Position after a2a4 (should be Black to move):" << std::endl;
    std::cout << "Side to move: " << ((int)pos.side_to_move ? "BLACK" : "WHITE") << std::endl;
    
    std::cout << "\nRunning perft(1) from position after a2a4:" << std::endl;
    uint64_t perft1 = perft_legal(pos, 1);
    std::cout << "Perft(1) result: " << perft1 << " (should be 20 for any valid position)" << std::endl;
    
    std::cout << "\nRunning detailed perft(1) breakdown from position after a2a4:" << std::endl;
    uint64_t perft1_detailed = perft_divide_detailed(pos, 1, "  ");
    std::cout << "Total from detailed breakdown: " << perft1_detailed << std::endl;
    
    // Unmake the move
    pos.unmake_move(a2a4, undo_info);
    
    std::cout << "\nAfter unmaking a2a4, back to starting position:" << std::endl;
    std::cout << "Side to move: " << ((int)pos.side_to_move ? "BLACK" : "WHITE") << std::endl;
    
    std::cout << "\nRunning full a2a4 perft(2) test:" << std::endl;
    std::cout << "This should give 420 nodes for a correct implementation" << std::endl;
    
    // Test the full perft(2) calculation for a2a4
    if (pos.is_legal_move(a2a4)) {
        BitboardPosition::UndoInfo undo_info2 = pos.make_move_with_undo(a2a4);
        uint64_t a2a4_perft2 = perft_legal(pos, 2);
        pos.unmake_move(a2a4, undo_info2);
        
        std::cout << "a2a4 perft(2): " << a2a4_perft2 << std::endl;
        std::cout << "Expected: 420" << std::endl;
        std::cout << "Difference: " << (int64_t)a2a4_perft2 - 420 << std::endl;
        
        if (a2a4_perft2 != 420) {
            std::cout << "\nDetailed analysis - making a2a4 and running perft(1) breakdown:" << std::endl;
            BitboardPosition::UndoInfo undo_info3 = pos.make_move_with_undo(a2a4);
            uint64_t detailed_result = perft_divide_detailed(pos, 1, "  ");
            pos.unmake_move(a2a4, undo_info3);
            std::cout << "Sum of all moves from a2a4 position: " << detailed_result << std::endl;
        }
    }
    
    return 0;
}