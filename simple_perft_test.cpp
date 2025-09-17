/**
 * @file simple_perft_test.cpp
 * @brief Simple perft test for both positions
 */

#include <iostream>
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
        if (pos.make_move(simple_move)) {
            nodes += perft_original(pos, depth - 1);
            pos.unmake_move(simple_move);
        }
    }
    return nodes;
}

uint64_t perft_optimized(const BitboardPosition& pos, int depth) {
    return BitboardPerftOptimized::perft_fast(pos, depth);
}

int main() {
    cout << "=== Simple Perft Test ===" << endl;
    
    // Initialize engine
    Huginn::init();
    if (!Huginn::is_initialized()) {
        cout << "ERROR: Failed to initialize engine" << endl;
        return 1;
    }
    
    // Starting position tests
    cout << "\n=== Starting Position ===" << endl;
    BitboardPosition start_pos;
    if (!start_pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "Failed to parse starting position" << endl;
        return 1;
    }
    
    for (int depth = 1; depth <= 6; ++depth) {
        uint64_t orig = perft_original(start_pos, depth);
        uint64_t opt = perft_optimized(start_pos, depth);
        int diff = (int)opt - (int)orig;
        
        cout << "Depth " << depth << ": Original " << orig << ", Optimized " << opt;
        if (diff != 0) {
            cout << " (diff: " << diff << ")";
        }
        cout << endl;
    }
    
    // Kiwipete tests
    cout << "\n=== Kiwipete Position ===" << endl;
    BitboardPosition kiwi_pos;
    if (!kiwi_pos.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1")) {
        cout << "Failed to parse Kiwipete position" << endl;
        return 1;
    }
    
    for (int depth = 1; depth <= 5; ++depth) {
        uint64_t orig = perft_original(kiwi_pos, depth);
        uint64_t opt = perft_optimized(kiwi_pos, depth);
        int diff = (int)opt - (int)orig;
        
        cout << "Depth " << depth << ": Original " << orig << ", Optimized " << opt;
        if (diff != 0) {
            cout << " (diff: " << diff << ")";
        }
        cout << endl;
    }
    
    return 0;
}