/**
 * @file check_move_order_pattern.cpp
 * @brief Check if errors start at a specific move number
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>

using namespace BitboardMoveGen;

// Helper function to convert move for compatibility
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

// Simple perft function
uint64_t perft(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        total_nodes += perft(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }

    return total_nodes;
}

int main() {
    std::cout << "=== MOVE ORDER PATTERN ANALYSIS ===" << std::endl;
    
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "FEN: " << fen << std::endl;
    
    // Reference values from known correct implementation
    std::map<std::string, uint64_t> reference = {
        {"a1b1", 3827454}, {"a1c1", 3814203}, {"a1d1", 3568344},
        {"a2a3", 4627439}, {"a2a4", 4387586}, {"b2b3", 3768824},
        {"c3a4", 4628497}, {"c3b1", 3996171}, {"c3b5", 4317482}, {"c3d1", 3995761},
        {"d2c1", 3793390}, {"d2e3", 4407041}, {"d2f4", 3941257}, {"d2g5", 4370915}, {"d2h6", 3967365},
        {"d5d6", 3835265}, {"d5e6", 4727437},
        {"e1c1", 3551583}, {"e1d1", 3559113}, {"e1f1", 3377351}, {"e1g1", 4119629},
        {"e2a6", 3553501}, {"e2b5", 4032348}, {"e2c4", 4182989}, {"e2d1", 3074219}, {"e2d3", 4066966}, {"e2f1", 4095479},
        {"e5c4", 3494887}, {"e5c6", 4083458}, {"e5d3", 3288812}, {"e5d7", 4404043}, {"e5f7", 4164923}, {"e5g4", 3415992}, {"e5g6", 3949417},
        {"f3d3", 3949570}, {"f3e3", 4477772}, {"f3f4", 4327936}, {"f3f5", 5271134}, {"f3f6", 3975992}, {"f3g3", 4669768}, {"f3g4", 4514010}, {"f3h3", 5067173}, {"f3h5", 4743335},
        {"g2g3", 3472039}, {"g2g4", 3338154}, {"g2h3", 3819456},
        {"h1f1", 3685756}, {"h1g1", 3989454}
    };
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cerr << "Failed to parse FEN!" << std::endl;
        return 1;
    }
    
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    std::cout << "\nMove-by-move analysis (in generation order):" << std::endl;
    std::cout << "# | Move  | Expected | Actual   | Difference | Status" << std::endl;
    std::cout << "--|-------|----------|----------|------------|--------" << std::endl;
    
    int correct_streak = 0;
    bool streak_broken = false;
    int first_error_move = -1;
    
    for (size_t i = 0; i < moves.moves.size(); i++) {
        SimpleBitboardMove simple_move = convert_move(moves.moves[i]);
        std::string move_str = move_to_string(simple_move);
        
        BitboardPosition pos_copy = pos;
        BitboardPosition::UndoInfo undo = pos_copy.make_move_with_undo(simple_move);
        uint64_t actual = perft(pos_copy, 4);  // Depth 5 total = depth 4 after move
        pos_copy.unmake_move(simple_move, undo);
        
        auto it = reference.find(move_str);
        if (it != reference.end()) {
            uint64_t expected = it->second;
            int64_t diff = (int64_t)actual - (int64_t)expected;
            
            std::string status;
            if (actual == expected) {
                status = "CORRECT";
                if (!streak_broken) {
                    correct_streak++;
                }
            } else {
                status = (diff < 0) ? "UNDER" : "OVER";
                if (!streak_broken) {
                    streak_broken = true;
                    first_error_move = i + 1;
                }
            }
            
            std::cout << std::setw(2) << (i+1)
                      << "| " << std::setw(5) << move_str
                      << " | " << std::setw(8) << expected
                      << " | " << std::setw(8) << actual
                      << " | " << std::setw(10) << diff
                      << " | " << status << std::endl;
        } else {
            std::cout << std::setw(2) << (i+1) << "| " << move_str << " | NOT FOUND IN REFERENCE" << std::endl;
        }
    }
    
    std::cout << "\n=== PATTERN ANALYSIS ===" << std::endl;
    std::cout << "Correct streak from start: " << correct_streak << " moves" << std::endl;
    if (first_error_move > 0) {
        std::cout << "First error at move #: " << first_error_move << std::endl;
    } else {
        std::cout << "All moves were correct!" << std::endl;
    }
    
    if (correct_streak == 10) {
        std::cout << "✅ YES! The first 10 moves were correct, then errors started." << std::endl;
    } else {
        std::cout << "❌ NO! The pattern is not 'first 10 correct, then errors'." << std::endl;
        std::cout << "Actually: first " << correct_streak << " moves were correct." << std::endl;
    }
    
    return 0;
}