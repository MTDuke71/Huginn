#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
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

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

// Perft function
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        BitboardPosition copy = pos;
        if (copy.make_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            nodes += perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
        }
    }
    
    return nodes;
}

int main() {
    std::cout << "=== Find Problematic Move ===\n\n";
    
    // Expected results from working output
    std::map<std::string, int> expected = {
        {"a1b1", 43}, {"a1c1", 43}, {"a1d1", 43}, {"a2a3", 44}, {"a2a4", 44},
        {"b2b3", 42}, {"c3a4", 42}, {"c3b1", 42}, {"c3b5", 39}, {"c3d1", 42},
        {"d2c1", 43}, {"d2e3", 43}, {"d2f4", 43}, {"d2g5", 42}, {"d2h6", 41},
        {"d5d6", 41}, {"d5e6", 46}, {"e1c1", 43}, {"e1d1", 43}, {"e1f1", 43},
        {"e1g1", 43}, {"e2a6", 36}, {"e2b5", 39}, {"e2c4", 41}, {"e2d1", 44},
        {"e2d3", 42}, {"e2f1", 44}, {"e5c4", 42}, {"e5c6", 41}, {"e5d3", 43},
        {"e5d7", 45}, {"e5f7", 44}, {"e5g4", 44}, {"e5g6", 42}, {"f3d3", 42},
        {"f3e3", 43}, {"f3f4", 43}, {"f3f5", 45}, {"f3f6", 39}, {"f3g3", 43},
        {"f3g4", 43}, {"f3h3", 43}, {"f3h5", 43}, {"g2g3", 42}, {"g2g4", 42},
        {"g2h3", 43}, {"h1f1", 43}, {"h1g1", 43}
    };
    
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Testing position: " << fen << "\n\n";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << "Move        Expected  Our Result  Difference\n";
    std::cout << "----------------------------------------------\n";
    
    int total_extra = 0;
    std::vector<std::string> problematic_moves;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        BitboardPosition copy = pos;
        if (copy.make_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t our_result = perft_legal(pos, 1);
            pos.unmake_move(simple_move, undo);
            
            int expected_result = expected[move_str];
            int difference = static_cast<int>(our_result) - expected_result;
            
            std::cout << std::setw(8) << move_str 
                      << std::setw(10) << expected_result
                      << std::setw(12) << our_result
                      << std::setw(12) << difference;
            
            if (difference != 0) {
                std::cout << " <-- PROBLEM";
                total_extra += difference;
                problematic_moves.push_back(move_str);
            }
            
            std::cout << "\n";
        }
    }
    
    std::cout << "\n=== Summary ===\n";
    std::cout << "Total extra moves: " << total_extra << "\n";
    std::cout << "Problematic moves: ";
    for (size_t i = 0; i < problematic_moves.size(); ++i) {
        std::cout << problematic_moves[i];
        if (i < problematic_moves.size() - 1) std::cout << ", ";
    }
    std::cout << "\n";
    
    return 0;
}