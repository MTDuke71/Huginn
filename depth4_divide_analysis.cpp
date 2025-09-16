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

// Move to string function
std::string move_to_string(const SimpleBitboardMove& move) {
    std::string result;
    result += char('a' + (move.from_64 % 8));
    result += char('1' + (move.from_64 / 8));
    result += char('a' + (move.to_64 % 8));
    result += char('1' + (move.to_64 / 8));
    return result;
}

// Perft function 
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            nodes += perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
        }
    }
    
    return nodes;
}

int main() {
    std::cout << "=== Starting Position Depth 4 Analysis ===\n\n";
    
    // Reference data from user
    std::map<std::string, uint64_t> expected_depth4 = {
        {"a2a3", 8457}, {"a2a4", 9329}, {"b1a3", 8885}, {"b1c3", 9755}, {"b2b3", 9345},
        {"b2b4", 9332}, {"c2c3", 9272}, {"c2c4", 9744}, {"d2d3", 11959}, {"d2d4", 12435},
        {"e2e3", 13134}, {"e2e4", 13160}, {"f2f3", 8457}, {"f2f4", 8929}, {"g1f3", 9748},
        {"g1h3", 8881}, {"g2g3", 9345}, {"g2g4", 9328}, {"h2h3", 8457}, {"h2h4", 9329}
    };
    
    // Start from the initial position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing perft divide 4 from starting position:\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t total_nodes = 0;
    uint64_t expected_total = 197281;
    bool found_errors = false;
    
    std::cout << "Move    Our Count   Expected    Difference  Status\n";
    std::cout << "------------------------------------------------------------\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = move_to_string(simple_move);
            
            BitboardPosition::UndoInfo move_undo = pos.make_move_with_undo(simple_move);
            uint64_t count = perft_legal(pos, 3);  // Depth 4 total = depth 3 after first move
            pos.unmake_move(simple_move, move_undo);
            
            auto it = expected_depth4.find(move_str);
            if (it != expected_depth4.end()) {
                uint64_t expected_count = it->second;
                int64_t diff = (int64_t)count - (int64_t)expected_count;
                
                std::cout << move_str << "    " << std::setw(8) << count << "    " << std::setw(8) << expected_count;
                std::cout << "    " << std::setw(8) << diff;
                
                if (diff == 0) {
                    std::cout << "       ✅\n";
                } else {
                    std::cout << "       ❌";
                    if (diff > 0) {
                        std::cout << " (OVER-GEN)";
                    } else {
                        std::cout << " (UNDER-GEN)";
                    }
                    std::cout << "\n";
                    found_errors = true;
                }
                
                total_nodes += count;
            } else {
                std::cout << move_str << "    " << std::setw(8) << count << "    UNKNOWN     UNKNOWN        ?\n";
                total_nodes += count;
            }
        }
    }
    
    std::cout << "------------------------------------------------------------\n";
    int64_t total_diff = (int64_t)total_nodes - (int64_t)expected_total;
    std::cout << "TOTAL   " << std::setw(8) << total_nodes << "    " << std::setw(8) << expected_total;
    std::cout << "    " << std::setw(8) << total_diff;
    
    if (total_diff == 0) {
        std::cout << "       ✅\n";
    } else {
        std::cout << "       ❌";
        if (total_diff > 0) {
            std::cout << " (OVER-GEN)";
        } else {
            std::cout << " (UNDER-GEN)";
        }
        std::cout << "\n";
        found_errors = true;
    }
    
    std::cout << "\n=== Analysis Summary ===\n";
    if (found_errors) {
        std::cout << "❌ Found discrepancies at depth 4!\n";
        std::cout << "Total error: " << total_diff << " nodes\n";
        if (total_diff > 0) {
            std::cout << "We are OVER-generating moves (invalid moves passing legal check)\n";
        } else {
            std::cout << "We are UNDER-generating moves (missing legal moves)\n";
        }
        std::cout << "Focus on moves with errors to identify the bug.\n";
    } else {
        std::cout << "✅ Perfect match. All moves correct at depth 4.\n";
    }
    
    return 0;
}