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

// Convert square to algebraic for en passant display
std::string square_to_alg(int square) {
    if (square == -1) return "-";
    return std::string(1, 'a' + (square % 8)) + std::string(1, '1' + (square / 8));
}

int main() {
    std::cout << "=== Analysis After a2a4 a7a6 a4a5 ===\n\n";
    
    // Reference data from user after a2a4 a7a6 a4a5
    std::map<std::string, uint64_t> expected_after_a4a5 = {
        {"a8a7", 21}, {"b7b5", 22}, {"b7b6", 22}, {"b8c6", 21}, {"c7c5", 21},
        {"c7c6", 21}, {"d7d5", 21}, {"d7d6", 21}, {"e7e5", 21}, {"e7e6", 21},
        {"f7f5", 21}, {"f7f6", 21}, {"g7g5", 21}, {"g7g6", 21}, {"g8f6", 21},
        {"g8h6", 21}, {"h7h5", 21}, {"h7h6", 21}
    };
    
    // Start from the initial position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Playing sequence: a2a4 a7a6 a4a5\n\n";
    
    // Make a2a4 move (white pawn 2-square push)
    SimpleBitboardMove a2a4;
    a2a4.from_64 = 8;  // a2 square
    a2a4.to_64 = 24;   // a4 square
    a2a4.is_capture = false;
    a2a4.is_ep_capture = false;
    a2a4.is_castling = false;
    a2a4.is_promotion = false;
    a2a4.promotion_type = PieceType::None;
    
    BitboardPosition::UndoInfo undo1 = pos.make_move_with_undo(a2a4);
    std::cout << "After a2a4: ep_square = " << square_to_alg(pos.ep_square_64) << "\n";
    
    // Make a7a6 move (black pawn 1-square push)
    SimpleBitboardMove a7a6;
    a7a6.from_64 = 48;  // a7 square
    a7a6.to_64 = 40;    // a6 square
    a7a6.is_capture = false;
    a7a6.is_ep_capture = false;
    a7a6.is_castling = false;
    a7a6.is_promotion = false;
    a7a6.promotion_type = PieceType::None;
    
    BitboardPosition::UndoInfo undo2 = pos.make_move_with_undo(a7a6);
    std::cout << "After a7a6: ep_square = " << square_to_alg(pos.ep_square_64) << "\n";
    
    // Make a4a5 move (white pawn 1-square push)
    SimpleBitboardMove a4a5;
    a4a5.from_64 = 24;  // a4 square
    a4a5.to_64 = 32;    // a5 square
    a4a5.is_capture = false;
    a4a5.is_ep_capture = false;
    a4a5.is_castling = false;
    a4a5.is_promotion = false;
    a4a5.promotion_type = PieceType::None;
    
    BitboardPosition::UndoInfo undo3 = pos.make_move_with_undo(a4a5);
    std::cout << "After a4a5: ep_square = " << square_to_alg(pos.ep_square_64) << "\n\n";
    
    std::cout << "Final position after a2a4 a7a6 a4a5:\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    std::cout << "White pawn on a5: " << (pos.piece_at(32) != Piece::None ? "✅" : "❌") << "\n";
    std::cout << "Black pawn on a6: " << (pos.piece_at(40) != Piece::None ? "✅" : "❌") << "\n";
    std::cout << "En passant square: " << square_to_alg(pos.ep_square_64) << "\n\n";
    
    // Now test perft divide 2 from this position
    std::cout << "Testing perft divide 2 after a2a4 a7a6 a4a5:\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t total_nodes = 0;
    uint64_t expected_total = 380;
    bool found_errors = false;
    
    std::cout << "Move    Our Count   Expected    Difference  Status\n";
    std::cout << "------------------------------------------------------------\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = move_to_string(simple_move);
            
            BitboardPosition::UndoInfo move_undo = pos.make_move_with_undo(simple_move);
            uint64_t count = perft_legal(pos, 1);  // Depth 2 total = depth 1 after first move
            pos.unmake_move(simple_move, move_undo);
            
            auto it = expected_after_a4a5.find(move_str);
            if (it != expected_after_a4a5.end()) {
                uint64_t expected_count = it->second;
                int64_t diff = (int64_t)count - (int64_t)expected_count;
                
                std::cout << move_str << "    " << std::setw(8) << count << "    " << std::setw(8) << expected_count;
                std::cout << "    " << std::setw(8) << diff;
                
                if (diff == 0) {
                    std::cout << "       ✅\n";
                } else {
                    std::cout << "       ❌\n";
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
        std::cout << "       ❌\n";
        found_errors = true;
    }
    
    std::cout << "\n=== Analysis Summary ===\n";
    if (found_errors) {
        std::cout << "❌ Found the exact bug location!\n";
        std::cout << "The error occurs in the position after a2a4 a7a6 a4a5.\n";
        std::cout << "This is a very specific pawn configuration issue.\n";
    } else {
        std::cout << "✅ Perfect match. The bug manifests in deeper levels.\n";
    }
    
    std::cout << "\n=== Position Analysis ===\n";
    std::cout << "Current position has:\n";
    std::cout << "- White pawn on a5 (advanced)\n";
    std::cout << "- Black pawn on a6 (adjacent)\n";
    std::cout << "- These pawns are next to each other on the a-file\n";
    std::cout << "- Black to move with various pawn and piece options\n";
    
    return 0;
}