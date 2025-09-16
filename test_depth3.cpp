#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

using namespace BitboardMoveGen;

// Simple perft function for testing
uint64_t perft(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move;
        simple_move.from_64 = move.from_64;
        simple_move.to_64 = move.to_64;
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        simple_move.promotion_type = move.promotion_type;

        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        nodes += perft(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }
    return nodes;
}

int main() {
    std::cout << "=== Kiwipete Depth 3 Test ===\n\n";

    // Kiwipete position FEN
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Position: " << fen << "\n\n";

    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }

    // Test depths 1, 2, 3
    for (int depth = 1; depth <= 3; depth++) {
        uint64_t result = perft(pos, depth);
        uint64_t expected = (depth == 1) ? 48 : (depth == 2) ? 2039 : 97862;

        std::cout << "Depth " << depth << ": " << result;
        if (result == expected) {
            std::cout << " ✅ CORRECT\n";
        } else {
            std::cout << " ❌ Expected: " << expected << " (diff: " << (int64_t)result - (int64_t)expected << ")\n";
        }
    }

    return 0;
}