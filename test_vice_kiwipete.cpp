#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/init.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>

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

// Perft divide for analysis
uint64_t perft_divide(Position& pos, int depth, bool print_moves = false) {
    if (depth == 0) return 1;

    S_MOVELIST list;
    generate_all_moves(pos, list);
    uint64_t total_nodes = 0;

    for (int i = 0; i < list.count; i++) {
        const auto& m = list.moves[i];
        if (pos.MakeMove(m) == 1) {
            uint64_t nodes = perft_vice(pos, depth - 1);
            pos.TakeMove();

            if (print_moves) {
                // Convert move to string for printing
                std::string move_str = moveToString(m);
                std::cout << move_str << " - " << nodes << "\n";
            }

            total_nodes += nodes;
        }
    }

    return total_nodes;
}

int main() {
    // Initialize the engine systems
    init::init_all();

    std::cout << "=== VICE Kiwipete Perft Test ===\n\n";

    // Kiwipete position FEN
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Position: " << fen << "\n\n";

    Position pos;
    if (!pos.parseFEN(fen)) {
        std::cout << "ERROR: Failed to parse FEN!\n";
        return 1;
    }

    // Test specific depths
    uint64_t expected_results[] = { 0, 48, 2039, 97862, 4085603, 193690690 };

    std::cout << "Depth   Our Result      Expected        Difference   Status      Time\n";
    std::cout << "------------------------------------------------------------------------\n";

    for (int depth = 1; depth <= 4; depth++) {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t result = perft_vice(pos, depth);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        uint64_t expected = expected_results[depth];
        int64_t difference = static_cast<int64_t>(result) - static_cast<int64_t>(expected);

        std::cout << std::setw(2) << depth
                  << std::setw(16) << result
                  << std::setw(16) << expected
                  << std::setw(16) << difference;

        if (difference == 0) {
            std::cout << "    ✅ CORRECT";
        } else {
            std::cout << "    ❌ ERROR  ";
        }

        std::cout << std::setw(8) << duration.count() << "ms\n";
    }

    std::cout << "\n=== CONCLUSION ===\n";
    std::cout << "If VICE gets correct results, then the bug is specifically in:\n";
    std::cout << "1. BitboardPosition class implementation\n";
    std::cout << "2. generate_legal_moves() function\n";
    std::cout << "3. Bitboard make_move_with_undo/unmake_move methods\n\n";

    std::cout << "The fix is to either:\n";
    std::cout << "A) Fix the BitboardPosition implementation\n";
    std::cout << "B) Use VICE Position for perft calculations\n";

    return 0;
}