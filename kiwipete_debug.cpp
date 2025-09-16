#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

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

int main() {
    std::cout << "=== Kiwipete Move Validation Debug ===\n\n";
    
    // Kiwipete position FEN
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Testing position: " << fen << "\n\n";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << "Total pseudo-legal moves generated: " << moves.moves.size() << "\n\n";
    
    std::vector<std::string> valid_moves;
    std::vector<std::string> invalid_moves;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        // Test with copy
        BitboardPosition copy = pos;
        if (copy.make_move(simple_move)) {
            valid_moves.push_back(move_str);
        } else {
            invalid_moves.push_back(move_str);
        }
    }
    
    std::sort(valid_moves.begin(), valid_moves.end());
    std::sort(invalid_moves.begin(), invalid_moves.end());
    
    std::cout << "VALID moves (" << valid_moves.size() << "):\n";
    for (const auto& move : valid_moves) {
        std::cout << "  " << move << "\n";
    }
    
    if (!invalid_moves.empty()) {
        std::cout << "\nINVALID moves (" << invalid_moves.size() << "):\n";
        for (const auto& move : invalid_moves) {
            std::cout << "  " << move << "\n";
        }
    }
    
    std::cout << "\n=== Summary ===\n";
    std::cout << "Generated: " << moves.moves.size() << " pseudo-legal moves\n";
    std::cout << "Valid: " << valid_moves.size() << " moves\n";
    std::cout << "Invalid: " << invalid_moves.size() << " moves\n";
    std::cout << "Expected: 48 moves\n";
    
    if (valid_moves.size() == 48) {
        std::cout << "✓ CORRECT: All moves validated successfully!\n";
    } else {
        std::cout << "❌ ERROR: Missing " << (48 - valid_moves.size()) << " moves\n";
    }
    
    return 0;
}