/**
 * @file debug_perft_comprehensive.cpp
 * @brief Comprehensive perft debugging to find exact discrepancies
 */

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

string move_to_string(int from_64, int to_64) {
    string result;
    result += char('a' + (from_64 % 8));
    result += char('1' + (from_64 / 8));
    result += char('a' + (to_64 % 8));
    result += char('1' + (to_64 / 8));
    return result;
}

uint64_t perft_original(BitboardPosition pos, int depth) {
    if (depth == 0) return 1;
    
    if (depth == 1) {
        BitboardMoveList moves;
        BitboardMoveGen::generate_legal_moves(pos, moves);
        return moves.moves.size();
    }
    
    BitboardMoveList moves;
    BitboardMoveGen::generate_legal_moves(pos, moves);
    
    uint64_t total = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move(move.from_64, move.to_64, move.promotion_type);
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        
        BitboardPosition new_pos = pos;
        if (new_pos.make_move(simple_move)) {
            total += perft_original(new_pos, depth - 1);
        }
    }
    return total;
}

uint64_t perft_optimized(BitboardPosition pos, int depth) {
    if (depth == 0) return 1;
    
    if (depth == 1) {
        BitboardMoveList moves;
        BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);
        return moves.moves.size();
    }
    
    BitboardMoveList moves;
    BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);
    
    uint64_t total = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move(move.from_64, move.to_64, move.promotion_type);
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        
        BitboardPosition new_pos = pos;
        if (new_pos.make_move(simple_move)) {
            total += perft_optimized(new_pos, depth - 1);
        }
    }
    return total;
}

void debug_depth3_starting_position() {
    cout << "=== Debugging Depth 3 - Starting Position ===" << endl;
    
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "Failed to parse starting position" << endl;
        return;
    }
    
    // Get moves from both methods
    BitboardMoveList original_moves, optimized_moves;
    
    BitboardPosition pos_copy = pos;
    BitboardMoveGen::generate_legal_moves(pos_copy, original_moves);
    BitboardPerftOptimized::generate_legal_moves_fast(pos, optimized_moves);
    
    cout << "Depth 1: Original " << original_moves.moves.size() << ", Optimized " << optimized_moves.moves.size() << endl;
    
    uint64_t original_total = 0;
    uint64_t optimized_total = 0;
    
    cout << "\nMove-by-move depth 3 analysis:" << endl;
    cout << "Move                Original  Optimized  Diff" << endl;
    cout << "----------------------------------------------" << endl;
    
    map<string, pair<uint64_t, uint64_t>> move_results;
    
    // Process original moves
    for (const auto& move : original_moves.moves) {
        SimpleBitboardMove simple_move(move.from_64, move.to_64, move.promotion_type);
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        
        BitboardPosition new_pos = pos;
        if (new_pos.make_move(simple_move)) {
            uint64_t nodes = perft_original(new_pos, 2);
            string move_str = move_to_string(move.from_64, move.to_64);
            move_results[move_str].first = nodes;
            original_total += nodes;
        }
    }
    
    // Process optimized moves
    for (const auto& move : optimized_moves.moves) {
        SimpleBitboardMove simple_move(move.from_64, move.to_64, move.promotion_type);
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        
        BitboardPosition new_pos = pos;
        if (new_pos.make_move(simple_move)) {
            uint64_t nodes = perft_optimized(new_pos, 2);
            string move_str = move_to_string(move.from_64, move.to_64);
            move_results[move_str].second = nodes;
            optimized_total += nodes;
        }
    }
    
    // Display results
    for (const auto& [move_str, results] : move_results) {
        uint64_t orig = results.first;
        uint64_t opt = results.second;
        int64_t diff = (int64_t)opt - (int64_t)orig;
        
        cout << setw(16) << move_str << setw(8) << orig << setw(10) << opt << setw(6) << diff;
        if (diff != 0) {
            cout << " <-- MISMATCH";
        }
        cout << endl;
    }
    
    cout << "----------------------------------------------" << endl;
    cout << "Total depth 3:      " << setw(8) << original_total << setw(10) << optimized_total << endl;
    cout << "Expected: 8902" << endl;
    cout << "Difference: " << (int64_t)optimized_total - (int64_t)original_total << endl;
}

void debug_depth2_kiwipete() {
    cout << "\n=== Debugging Depth 2 - Kiwipete ===" << endl;
    
    BitboardPosition pos;
    if (!pos.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1")) {
        cout << "Failed to parse Kiwipete" << endl;
        return;
    }
    
    // Test at depth 2 for Kiwipete where we saw 1996 vs 2039
    uint64_t orig_total = perft_original(pos, 2);
    uint64_t opt_total = perft_optimized(pos, 2);
    
    cout << "Kiwipete depth 2:" << endl;
    cout << "Original:  " << orig_total << endl;
    cout << "Optimized: " << opt_total << endl;
    cout << "Expected:  2039" << endl;
    cout << "Difference: " << (int64_t)opt_total - (int64_t)orig_total << endl;
}

int main() {
    cout << "Comprehensive Perft Debugging" << endl;
    cout << "=============================" << endl;
    
    try {
        Huginn::init();
        if (!Huginn::is_initialized()) {
            cout << "ERROR: Failed to initialize engine" << endl;
            return 1;
        }
        
        debug_depth3_starting_position();
        debug_depth2_kiwipete();
        
    } catch (const exception& e) {
        cout << "Exception: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}