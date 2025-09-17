/**
 * @file debug_recursive_consistency.cpp
 * @brief Check if recursive perft functions are consistent with themselves
 */

#include <iostream>
#include <iomanip>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

string square_to_string(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return string(1, 'a' + file) + string(1, '1' + rank);
}

string move_to_string(const BitboardMoveList::BitboardMove& move) {
    return square_to_string(move.from_64) + square_to_string(move.to_64);
}

// Separate perft implementations to verify consistency
uint64_t perft_orig_manual(BitboardPosition pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    BitboardMoveGen::generate_legal_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move(move.from_64, move.to_64);
        auto undo_info = pos.make_move_with_undo(simple_move);
        nodes += perft_orig_manual(pos, depth - 1);
        pos.unmake_move(simple_move, undo_info);
    }
    return nodes;
}

uint64_t perft_opt_manual(const BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        BitboardPosition test_pos = pos;
        SimpleBitboardMove simple_move(move.from_64, move.to_64);
        if (test_pos.make_move(simple_move)) {
            nodes += perft_opt_manual(test_pos, depth - 1);
        }
    }
    return nodes;
}

uint64_t perft_opt_manual_sum(const BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);
    
    cout << "  [Depth " << depth << "] Generated " << moves.moves.size() << " moves" << endl;
    
    uint64_t nodes = 0;
    for (size_t i = 0; i < moves.moves.size(); i++) {
        const auto& move = moves.moves[i];
        BitboardPosition test_pos = pos;
        SimpleBitboardMove simple_move(move.from_64, move.to_64);
        if (test_pos.make_move(simple_move)) {
            uint64_t move_nodes = perft_opt_manual(test_pos, depth - 1);
            nodes += move_nodes;
            cout << "    Move " << i << " (" << move_to_string(move) << "): " << move_nodes << " nodes" << endl;
        }
    }
    cout << "  [Depth " << depth << "] Total: " << nodes << " nodes" << endl;
    return nodes;
}

int main() {
    cout << "=== Recursive Consistency Check ===" << endl;
    
    // Initialize engine
    Huginn::init();
    if (!Huginn::is_initialized()) {
        cout << "ERROR: Failed to initialize engine" << endl;
        return 1;
    }
    
    // Test position: after e2e3
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1")) {
        cout << "Failed to parse test position" << endl;
        return 1;
    }
    
    cout << "Test Position (after e2e3): " << pos.to_fen() << endl << endl;
    
    // Test consistency at different depths
    for (int depth = 1; depth <= 3; depth++) {
        cout << "=== Testing Depth " << depth << " ===" << endl;
        
        uint64_t orig = perft_orig_manual(pos, depth);
        uint64_t opt1 = perft_opt_manual(pos, depth);
        
        cout << "Original perft: " << orig << endl;
        cout << "Optimized perft: " << opt1 << endl;
        
        if (depth <= 2) {  // Only show detailed for small depths
            cout << "\nDetailed optimized breakdown:" << endl;
            uint64_t opt2 = perft_opt_manual_sum(pos, depth);
            cout << "Sum verification: " << opt2 << endl;
            
            if (opt1 != opt2) {
                cout << "*** INCONSISTENCY IN OPTIMIZED METHOD! ***" << endl;
            }
        }
        
        int diff = (int)opt1 - (int)orig;
        cout << "Difference: " << diff << endl;
        
        if (diff != 0) {
            cout << "*** DISCREPANCY FOUND AT DEPTH " << depth << " ***" << endl;
        }
        
        cout << string(50, '-') << endl;
    }
    
    return 0;
}