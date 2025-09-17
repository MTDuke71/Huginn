#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

uint64_t test_make_unmake_method(const BitboardPosition& start_pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveGen movegen;
    BitboardMoveList moves;
    movegen.generate_legal_moves(const_cast<BitboardPosition&>(start_pos), moves);

    uint64_t nodes = 0;
    BitboardPosition pos = start_pos;  // Copy for modification

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move(move.from_64, move.to_64);

        // Method 1: make_move_with_undo + unmake_move
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
        nodes += test_make_unmake_method(pos, depth - 1);
        pos.unmake_move(simple_move, undo_info);
    }
    return nodes;
}

uint64_t test_copy_method(const BitboardPosition& start_pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveGen movegen;
    BitboardMoveList moves;
    movegen.generate_legal_moves(const_cast<BitboardPosition&>(start_pos), moves);

    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move(move.from_64, move.to_64);

        // Method 2: make_move on a copy
        BitboardPosition test_pos = start_pos;
        if (test_pos.make_move(simple_move)) {
            nodes += test_copy_method(test_pos, depth - 1);
        }
    }
    return nodes;
}

int main() {
    std::cout << "=== Testing Different Move Making Methods ===\n";

    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::cout << "Testing depth 1:\n";
    uint64_t method1_d1 = test_make_unmake_method(pos, 1);
    uint64_t method2_d1 = test_copy_method(pos, 1);
    std::cout << "  Make/unmake method: " << method1_d1 << "\n";
    std::cout << "  Copy method: " << method2_d1 << "\n";
    std::cout << "  Difference: " << (int64_t)method2_d1 - (int64_t)method1_d1 << "\n\n";

    std::cout << "Testing depth 2:\n";
    uint64_t method1_d2 = test_make_unmake_method(pos, 2);
    uint64_t method2_d2 = test_copy_method(pos, 2);
    std::cout << "  Make/unmake method: " << method1_d2 << "\n";
    std::cout << "  Copy method: " << method2_d2 << "\n";
    std::cout << "  Difference: " << (int64_t)method2_d2 - (int64_t)method1_d2 << "\n\n";

    std::cout << "Testing depth 3:\n";
    uint64_t method1_d3 = test_make_unmake_method(pos, 3);
    uint64_t method2_d3 = test_copy_method(pos, 3);
    std::cout << "  Make/unmake method: " << method1_d3 << "\n";
    std::cout << "  Copy method: " << method2_d3 << "\n";
    std::cout << "  Difference: " << (int64_t)method2_d3 - (int64_t)method1_d3 << "\n";

    return 0;
}