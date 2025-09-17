/**
 * @file test_starting_position_depths.cpp
 * @brief Test starting position perft for depths 1-6 after wrap-around fixes
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
        SimpleBitboardMove simple_move(move.from_64, move.to_64);
        auto undo_info = pos.make_move_with_undo(simple_move);
        nodes += perft_original(pos, depth - 1);
        pos.unmake_move(simple_move, undo_info);
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
    cout << "=== Starting Position Depths 1-6 Test ===" << endl;
    
    // Initialize engine
    Huginn::init();
    if (!Huginn::is_initialized()) {
        cout << "ERROR: Failed to initialize engine" << endl;
        return 1;
    }
    
    // Set up starting position
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "Failed to parse starting position" << endl;
        return 1;
    }
    
    // Expected node counts for starting position
    vector<uint64_t> expected = {0, 20, 400, 8902, 197281, 4865609, 119060324};
    
    cout << "Starting Position FEN: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" << endl << endl;
    
    cout << "Depth   Expected    Original    Optimized   Orig-Diff   Opt-Diff" << endl;
    cout << "------  ----------  ----------  ----------  ---------   --------" << endl;
    
    bool all_correct = true;
    
    for (int depth = 1; depth <= 6; ++depth) {
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
        if (depth >= 4 && (orig_diff != 0 || opt_diff != 0)) {
            cout << "\nStopping due to errors at depth " << depth << endl;
            break;
        }
    }
    
    cout << endl;
    if (all_correct) {
        cout << "🎉 ALL TESTS PASSED! Starting position perft is correct for all depths." << endl;
    } else {
        cout << "❌ Some tests failed. Need further debugging." << endl;
    }
    
    return all_correct ? 0 : 1;
}