/**
 * @file debug_starting_depth4.cpp
 * @brief Detailed analysis of starting position depth 4 discrepancies
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
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
    cout << "=== Starting Position Depth 4 Analysis ===" << endl;
    
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
    
    cout << "Starting Position FEN: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" << endl << endl;
    
    // Generate depth 1 moves and analyze each one
    BitboardMoveList orig_moves;
    BitboardMoveGen::generate_legal_moves(pos, orig_moves);
    
    cout << "Move-by-move depth 4 analysis:" << endl;
    cout << "Move       Original   Optimized   Diff" << endl;
    cout << "------     ---------  ---------   ----" << endl;
    
    uint64_t total_orig = 0, total_opt = 0;
    int moves_with_diff = 0;
    
    for (const auto& move : orig_moves.moves) {
        BitboardPosition test_pos = pos;
        SimpleBitboardMove simple_move(move.from_64, move.to_64);
        auto undo_info = test_pos.make_move_with_undo(simple_move);
        
        uint64_t orig_nodes = perft_original(test_pos, 3);  // depth 4 = 1 + 3
        uint64_t opt_nodes = perft_optimized_recursive(test_pos, 3);
        
        test_pos.unmake_move(simple_move, undo_info);
        
        total_orig += orig_nodes;
        total_opt += opt_nodes;
        
        int diff = (int)opt_nodes - (int)orig_nodes;
        if (diff != 0) moves_with_diff++;
        
        cout << setw(10) << move_to_string(move) << "  "
             << setw(9) << orig_nodes << "  "
             << setw(9) << opt_nodes << "  "
             << setw(4) << diff;
        
        if (diff != 0) {
            cout << "  ***";
        }
        cout << endl;
    }
    
    cout << "------     ---------  ---------   ----" << endl;
    cout << "Total:     " << setw(9) << total_orig << "  " << setw(9) << total_opt << "  " << setw(4) << ((int)total_opt - (int)total_orig) << endl;
    cout << "Expected:  " << setw(9) << 197281 << endl;
    cout << "Orig diff: " << setw(9) << ((int)total_orig - 197281) << endl;
    cout << "Opt diff:  " << setw(9) << ((int)total_opt - 197281) << endl << endl;
    
    cout << "Moves with differences: " << moves_with_diff << " out of " << orig_moves.moves.size() << endl;
    
    // Also check if move counts match at depth 1
    BitboardMoveList opt_moves;
    BitboardPerftOptimized::generate_legal_moves_fast(pos, opt_moves);
    
    cout << "\nDepth 1 move generation:" << endl;
    cout << "Original method: " << orig_moves.moves.size() << " moves" << endl;
    cout << "Optimized method: " << opt_moves.moves.size() << " moves" << endl;
    
    if (orig_moves.moves.size() != opt_moves.moves.size()) {
        cout << "WARNING: Different move counts at depth 1!" << endl;
    }
    
    return 0;
}