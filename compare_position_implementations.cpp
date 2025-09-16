#include "src/position.hpp"
#include "src/bitboard_position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/init.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace BitboardMoveGen;

// VICE perft function (working implementation)
static uint64_t perft_vice(Position& pos, int depth) {
    if (depth == 0) return 1;
    S_MOVELIST list;
    generate_all_moves(pos, list);
    uint64_t nodes = 0;
    for (int i = 0; i < list.count; i++) {
        const auto& m = list.moves[i];
        if (pos.MakeMove(m) == 1) {
            nodes += perft_vice(pos, depth - 1);
            pos.TakeMove();
        }
    }
    return nodes;
}

// Convert function for bitboard moves
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

// Bitboard perft function (buggy implementation)
static uint64_t perft_bitboard(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        nodes += perft_bitboard(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }
    return nodes;
}

int main() {
    // Initialize the engine systems
    init::init_all();

    std::cout << "=== Comparing Position Implementations ===\n\n";

    // Kiwipete position FEN
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Position: " << fen << "\n\n";

    // Test both implementations
    Position vice_pos;
    BitboardPosition bitboard_pos;

    if (!vice_pos.parseFEN(fen)) {
        std::cout << "ERROR: VICE Position failed to parse FEN!\n";
        return 1;
    }

    if (!bitboard_pos.set_from_fen(fen)) {
        std::cout << "ERROR: BitboardPosition failed to parse FEN!\n";
        return 1;
    }

    std::cout << "Depth   VICE Result     Bitboard Result  Difference   Status\n";
    std::cout << "----------------------------------------------------------------\n";

    // Expected results for Kiwipete
    uint64_t expected_results[] = { 0, 48, 2039, 97862, 4085603, 193690690 };

    for (int depth = 1; depth <= 4; depth++) {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t vice_result = perft_vice(vice_pos, depth);
        auto mid = std::chrono::high_resolution_clock::now();
        uint64_t bitboard_result = perft_bitboard(bitboard_pos, depth);
        auto end = std::chrono::high_resolution_clock::now();

        int64_t difference = static_cast<int64_t>(bitboard_result) - static_cast<int64_t>(vice_result);
        uint64_t expected = expected_results[depth];

        std::cout << std::setw(2) << depth
                  << std::setw(15) << vice_result
                  << std::setw(17) << bitboard_result
                  << std::setw(12) << difference;

        if (vice_result == expected && bitboard_result == expected) {
            std::cout << "    ✅ BOTH CORRECT";
        } else if (vice_result == expected) {
            std::cout << "    ❌ BITBOARD WRONG";
        } else if (bitboard_result == expected) {
            std::cout << "    ❌ VICE WRONG";
        } else {
            std::cout << "    ❌ BOTH WRONG";
        }

        auto vice_time = std::chrono::duration_cast<std::chrono::milliseconds>(mid - start);
        auto bitboard_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - mid);
        std::cout << " (VICE: " << vice_time.count() << "ms, BB: " << bitboard_time.count() << "ms)\n";
    }

    std::cout << "\n=== ANALYSIS ===\n";
    std::cout << "If VICE is correct and Bitboard is wrong, the bug is in:\n";
    std::cout << "1. BitboardPosition move generation (generate_legal_moves)\n";
    std::cout << "2. BitboardPosition make_move_with_undo/unmake_move\n";
    std::cout << "3. Conversion between move representations\n\n";

    std::cout << "If both are wrong, the bug might be in the FEN parsing or\n";
    std::cout << "in the fundamental position representation.\n";

    return 0;
}