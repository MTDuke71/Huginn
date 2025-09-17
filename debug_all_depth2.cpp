/**
 * @file debug_all_depth2.cpp
 * @brief Find which depth 1 move leads to the depth 3 discrepancy
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
        auto undo_info = pos.make_move_with_undo(SimpleBitboardMove(move.from_64, move.to_64));
        nodes += perft_original(pos, depth - 1);
        pos.unmake_move(SimpleBitboardMove(move.from_64, move.to_64), undo_info);
    }
    return nodes;
}

uint64_t perft_optimized(BitboardPosition pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);

    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        auto undo_info = pos.make_move_with_undo(SimpleBitboardMove(move.from_64, move.to_64));
        nodes += perft_optimized(pos, depth - 1);
        pos.unmake_move(SimpleBitboardMove(move.from_64, move.to_64), undo_info);
    }
    return nodes;
}

int main() {
    cout << "=== Find Depth 3 Discrepancy Source ===" << endl;

    Huginn::init();
    if (!Huginn::is_initialized()) {
        cout << "ERROR: Failed to initialize engine" << endl;
        return 1;
    }

    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "Failed to parse starting position" << endl;
        return 1;
    }

    // Get all first moves
    BitboardMoveList first_moves;
    BitboardMoveGen::generate_legal_moves(pos, first_moves);

    cout << "Checking depth 2 after each first move:" << endl;
    cout << "Move       Original   Optimized   Diff" << endl;
    cout << "------     ---------  ---------   ----" << endl;

    uint64_t total_orig = 0, total_opt = 0;

    for (const auto& move : first_moves.moves) {
        string move_str = move_to_string(move);
        BitboardPosition test_pos = pos;

        auto undo_info = test_pos.make_move_with_undo(SimpleBitboardMove(move.from_64, move.to_64));

        uint64_t orig = perft_original(test_pos, 2);
        uint64_t opt = perft_optimized(test_pos, 2);
        int diff = (int)opt - (int)orig;

        total_orig += orig;
        total_opt += opt;

        cout << setw(10) << move_str << " "
             << setw(9) << orig << " "
             << setw(9) << opt << " "
             << setw(4) << diff;

        if (diff != 0) cout << "  ***";
        cout << endl;

        test_pos.unmake_move(SimpleBitboardMove(move.from_64, move.to_64), undo_info);
    }

    cout << "------     ---------  ---------   ----" << endl;
    cout << "Total:   " << setw(9) << total_orig << " " << setw(9) << total_opt
         << " " << setw(4) << ((int)total_opt - (int)total_orig) << endl;

    return 0;
}