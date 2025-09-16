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

// Square to algebraic for en passant display
std::string square_to_alg(int square) {
    if (square == -1) return "-";
    return std::string(1, 'a' + (square % 8)) + std::string(1, '1' + (square / 8));
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
    std::cout << "=== After a2a3 a7a5 Depth 2 Analysis ===\n\n";
    
    // Reference data from user after a2a3 a7a5
    std::map<std::string, uint64_t> expected_after_a7a5 = {
        {"a1a2", 21}, {"a3a4", 20}, {"b1c3", 21}, {"b2b3", 21}, {"b2b4", 22},
        {"c2c3", 21}, {"c2c4", 21}, {"d2d3", 21}, {"d2d4", 21}, {"e2e3", 21},
        {"e2e4", 21}, {"f2f3", 21}, {"f2f4", 21}, {"g1f3", 21}, {"g1h3", 21},
        {"g2g3", 21}, {"g2g4", 21}, {"h2h3", 21}, {"h2h4", 21}
    };
    
    // Start from the initial position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Playing sequence: a2a3 a7a5\n\n";
    
    // Make a2a3 move
    SimpleBitboardMove a2a3;
    a2a3.from_64 = 8;   // a2
    a2a3.to_64 = 16;    // a3
    a2a3.is_capture = false;
    a2a3.is_ep_capture = false;
    a2a3.is_castling = false;
    a2a3.is_promotion = false;
    a2a3.promotion_type = PieceType::None;
    
    pos.make_move_with_undo(a2a3);
    std::cout << "After a2a3: ep_square = " << square_to_alg(pos.ep_square_64) << "\n";
    
    // Make a7a5 move (black pawn 2-square move)
    SimpleBitboardMove a7a5;
    a7a5.from_64 = 48;  // a7
    a7a5.to_64 = 32;    // a5
    a7a5.is_capture = false;
    a7a5.is_ep_capture = false;
    a7a5.is_castling = false;
    a7a5.is_promotion = false;
    a7a5.promotion_type = PieceType::None;
    
    pos.make_move_with_undo(a7a5);
    std::cout << "After a7a5: ep_square = " << square_to_alg(pos.ep_square_64) << "\n\n";
    
    std::cout << "Position after a2a3 a7a5:\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    std::cout << "White pawn on a3: " << (pos.piece_at(16) != Piece::None ? "✅" : "❌") << "\n";
    std::cout << "Black pawn on a5: " << (pos.piece_at(32) != Piece::None ? "✅" : "❌") << "\n";
    std::cout << "En passant square: " << square_to_alg(pos.ep_square_64) << "\n\n";
    
    // Critical analysis of en passant
    if (pos.ep_square_64 == 40) {  // a6 square
        std::cout << "✅ En passant square correctly set to a6\n";
        std::cout << "White should be able to play a3xa6 en passant capture.\n\n";
    } else {
        std::cout << "❌ En passant square incorrectly set (expected a6 = 40, got " << pos.ep_square_64 << ")\n\n";
    }
    
    // Now test perft divide 2 from this position
    std::cout << "Testing perft divide 2 after a2a3 a7a5:\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t total_nodes = 0;
    uint64_t expected_total = 399;
    bool found_errors = false;
    bool found_extra_moves = false;
    
    std::cout << "Move    Our Count   Expected    Difference  Status      Notes\n";
    std::cout << "--------------------------------------------------------------------------\n";
    
    // Track all moves we generate
    std::map<std::string, uint64_t> our_moves;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = move_to_string(simple_move);
            
            BitboardPosition::UndoInfo move_undo = pos.make_move_with_undo(simple_move);
            uint64_t count = perft_legal(pos, 1);  // Depth 2 total = depth 1 after first move
            pos.unmake_move(simple_move, move_undo);
            
            our_moves[move_str] = count;
        }
    }
    
    // Compare with expected moves
    for (const auto& our_move : our_moves) {
        std::string move_str = our_move.first;
        uint64_t count = our_move.second;
        
        auto it = expected_after_a7a5.find(move_str);
        if (it != expected_after_a7a5.end()) {
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
            std::cout << move_str << "    " << std::setw(8) << count << "    MISSING     +EXTRA         ❌ EXTRA!";
            
            // Check if this is an en passant move
            for (const auto& move : moves.moves) {
                SimpleBitboardMove simple_move = convert_move(move);
                if (pos.is_legal_move(simple_move) && move_to_string(simple_move) == move_str) {
                    if (simple_move.is_ep_capture) {
                        std::cout << "  EN PASSANT";
                    }
                    break;
                }
            }
            std::cout << "\n";
            
            total_nodes += count;
            found_extra_moves = true;
        }
    }
    
    std::cout << "--------------------------------------------------------------------------\n";
    int64_t total_diff = (int64_t)total_nodes - (int64_t)expected_total;
    std::cout << "TOTAL   " << std::setw(8) << total_nodes << "    " << std::setw(8) << expected_total;
    std::cout << "    " << std::setw(8) << total_diff;
    
    if (total_diff == 0) {
        std::cout << "       ✅\n";
    } else {
        std::cout << "       ❌ ";
        if (total_diff == 1 && found_extra_moves) {
            std::cout << "(EXTRA MOVE!)";
        }
        std::cout << "\n";
    }
    
    std::cout << "\n=== Critical Analysis ===\n";
    if (found_extra_moves) {
        std::cout << "🎯 FOUND THE BUG: We're generating an EXTRA move not in reference!\n";
        std::cout << "This extra move is likely an invalid en passant capture.\n";
    } else if (found_errors) {
        std::cout << "❌ Found move count discrepancies.\n";
    } else {
        std::cout << "✅ Perfect match - bug is deeper.\n";
    }
    
    return 0;
}