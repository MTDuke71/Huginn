#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
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
    std::cout << "=== Testing Move Generation Discrepancy ===\n\n";

    // Test both methods on Kiwipete position
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Position: " << fen << "\n\n";

    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN!\n";
        return 1;
    }

    // Method 1: generate_legal_moves() directly
    BitboardMoveList legal_moves;
    generate_legal_moves(pos, legal_moves);

    std::vector<std::string> method1_moves;
    for (const auto& move : legal_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        method1_moves.push_back(move_to_string(simple_move));
    }
    std::sort(method1_moves.begin(), method1_moves.end());

    // Method 2: generate_all_moves() + manual is_legal_move() filtering
    BitboardMoveList all_moves;
    generate_all_moves(pos, all_moves);

    std::vector<std::string> method2_moves;
    for (const auto& move : all_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            method2_moves.push_back(move_to_string(simple_move));
        }
    }
    std::sort(method2_moves.begin(), method2_moves.end());

    // Compare results
    std::cout << "Method 1 (generate_legal_moves): " << method1_moves.size() << " moves\n";
    std::cout << "Method 2 (generate_all_moves + filter): " << method2_moves.size() << " moves\n\n";

    if (method1_moves.size() != method2_moves.size()) {
        std::cout << "❌ DIFFERENT COUNTS! This confirms the bug.\n\n";
    } else {
        std::cout << "✅ Same counts, checking individual moves...\n\n";
    }

    // Find moves in method1 but not method2
    std::cout << "Moves in generate_legal_moves but NOT in generate_all_moves+filter:\n";
    bool found_differences = false;
    for (const auto& move : method1_moves) {
        if (std::find(method2_moves.begin(), method2_moves.end(), move) == method2_moves.end()) {
            std::cout << "  " << move << "\n";
            found_differences = true;
        }
    }
    if (!found_differences) {
        std::cout << "  (none)\n";
    }

    // Find moves in method2 but not method1
    std::cout << "\nMoves in generate_all_moves+filter but NOT in generate_legal_moves:\n";
    found_differences = false;
    for (const auto& move : method2_moves) {
        if (std::find(method1_moves.begin(), method1_moves.end(), move) == method1_moves.end()) {
            std::cout << "  " << move << "\n";
            found_differences = true;
        }
    }
    if (!found_differences) {
        std::cout << "  (none)\n";
    }

    std::cout << "\n=== DIAGNOSIS ===\n";
    if (method1_moves == method2_moves) {
        std::cout << "✅ Both methods produce identical results.\n";
        std::cout << "The bug must be elsewhere in the perft calculation.\n";
    } else {
        std::cout << "❌ CONFIRMED: The two move generation methods are inconsistent!\n";
        std::cout << "This explains the depth 4 perft discrepancy.\n";
        std::cout << "The bug is in the difference between generate_legal_moves() and generate_all_moves()+filtering.\n";
    }

    return 0;
}