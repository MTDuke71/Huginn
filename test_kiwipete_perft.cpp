/**
 * @file test_kiwipete_perft.cpp
 * @brief Test perft on Kiwipete position for depths 1-5
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

uint64_t perft_original(BitboardPosition pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    BitboardMoveGen::generate_legal_moves(pos, moves);

    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        auto undo_info = pos.make_move_with_undo(SimpleBitboardMove(move.from_64, move.to_64));
        nodes += perft_original(pos, depth - 1);
        pos.unmake_move(SimpleBitboardMove(move.from_64, move.to_64), undo_info);
    }
    return nodes;
}

uint64_t perft_optimized_recursive(const BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);

    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        BitboardPosition test_pos = pos;
        SimpleBitboardMove simple_move(move.from_64, move.to_64);
        if (test_pos.make_move(simple_move)) {
            nodes += perft_optimized_recursive(test_pos, depth - 1);
        }
    }
    return nodes;
}

int main() {
    cout << "=== Kiwipete Position Depths 1-5 Test ===" << endl;

    // Initialize engine
    Huginn::init();
    if (!Huginn::is_initialized()) {
        cout << "ERROR: Failed to initialize engine" << endl;
        return 1;
    }

    // Set up Kiwipete position
    BitboardPosition pos;
    if (!pos.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1")) {
        cout << "Failed to parse Kiwipete position" << endl;
        return 1;
    }

    // Expected node counts for Kiwipete position
    vector<uint64_t> expected = {0, 6, 264, 9467, 422333, 15833292, 706045033};

    cout << "Kiwipete FEN: r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1" << endl << endl;

    cout << "Depth   Expected    Original    Optimized   Orig-Diff   Opt-Diff" << endl;
    cout << "------  ----------  ----------  ----------  ---------   --------" << endl;

    bool all_correct = true;

    for (int depth = 1; depth <= 5; ++depth) {
        uint64_t exp = expected[depth];
        uint64_t orig = perft_original(pos, depth);
        uint64_t opt = perft_optimized_recursive(pos, depth);

        int orig_diff = (int)orig - (int)exp;
        int opt_diff = (int)opt - (int)exp;

        cout << "  " << depth << "     "
             << setw(10) << exp << "  "
             << setw(10) << orig << "  "
             << setw(10) << opt << "  "
             << setw(9) << orig_diff << "  "
             << setw(8) << opt_diff;

        if (orig_diff != 0 || opt_diff != 0) {
            cout << "  *** FAIL ***";
            all_correct = false;
        } else {
            cout << "  PASS";
        }
        cout << endl;

        // Stop early for deep searches if there are issues
        if (depth >= 3 && (abs(orig_diff) > 100 || abs(opt_diff) > 100)) {
            cout << "\nStopping due to large errors at depth " << depth << endl;
            break;
        }
    }

    cout << endl;
    if (all_correct) {
        cout << "🎉 ALL TESTS PASSED! Kiwipete perft is correct for all depths." << endl;
    } else {
        cout << "❌ Some tests failed. Need further debugging." << endl;
    }

    return all_correct ? 0 : 1;
}