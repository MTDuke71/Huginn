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
    std::cout << "=== After a2a3 Depth 3 Analysis ===\n\n";
    
    // Reference data from user after a2a3
    std::map<std::string, uint64_t> expected_after_a2a3 = {
        {"a7a5", 399}, {"a7a6", 361}, {"b7b5", 400}, {"b7b6", 399}, {"b8a6", 380},
        {"b8c6", 418}, {"c7c5", 419}, {"c7c6", 399}, {"d7d5", 532}, {"d7d6", 512},
        {"e7e5", 570}, {"e7e6", 569}, {"f7f5", 381}, {"f7f6", 361}, {"g7g5", 400},
        {"g7g6", 399}, {"g8f6", 418}, {"g8h6", 380}, {"h7h5", 399}, {"h7h6", 361}
    };
    
    // Start from the initial position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Playing sequence: a2a3\n\n";
    
    // Make a2a3 move (white pawn 1-square push)
    SimpleBitboardMove a2a3;
    a2a3.from_64 = 8;   // a2 square
    a2a3.to_64 = 16;    // a3 square
    a2a3.is_capture = false;
    a2a3.is_ep_capture = false;
    a2a3.is_castling = false;
    a2a3.is_promotion = false;
    a2a3.promotion_type = PieceType::None;
    
    BitboardPosition::UndoInfo undo1 = pos.make_move_with_undo(a2a3);
    std::cout << "After a2a3: side_to_move = " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    std::cout << "White pawn on a3: " << (pos.piece_at(16) != Piece::None ? "✅" : "❌") << "\n\n";
    
    // Now test perft divide 3 from this position
    std::cout << "Testing perft divide 3 after a2a3:\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t total_nodes = 0;
    uint64_t expected_total = 8457;
    bool found_errors = false;
    
    std::cout << "Move    Our Count   Expected    Difference  Status\n";
    std::cout << "------------------------------------------------------------\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = move_to_string(simple_move);
            
            BitboardPosition::UndoInfo move_undo = pos.make_move_with_undo(simple_move);
            uint64_t count = perft_legal(pos, 2);  // Depth 3 total = depth 2 after first move
            pos.unmake_move(simple_move, move_undo);
            
            auto it = expected_after_a2a3.find(move_str);
            if (it != expected_after_a2a3.end()) {
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
        std::cout << "❌ Found the specific location of the +1 bug!\n";
        std::cout << "Total error: " << total_diff << " nodes\n";
        if (total_diff == 1) {
            std::cout << "🎯 EXACT MATCH: This is where the +1 over-generation occurs!\n";
            std::cout << "Look for the move with +1 error to identify the bug.\n";
        }
    } else {
        std::cout << "✅ Perfect match. The bug manifests deeper.\n";
    }
    
    return 0;
}