/**
 * @file debug_counting_logic.cpp
 * @brief Debug the perft counting logic to see where the discrepancy arises
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"
#include "src/chess_types.hpp"

using namespace std;

// Helper function to convert square index to string
string square_to_string(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return string(1, 'a' + file) + string(1, '1' + rank);
}

// Manual perft that shows every step for original method
uint64_t debug_perft_original(BitboardPosition pos, int depth, const string& move_prefix = "", int level = 0) {
    if (depth == 0) return 1;
    
    if (depth == 1) {
        BitboardMoveList moves;
        BitboardMoveGen::generate_legal_moves(pos, moves);
        return moves.moves.size();
    }
    
    BitboardMoveList moves;
    BitboardMoveGen::generate_legal_moves(pos, moves);
    
    uint64_t total_nodes = 0;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move(move.from_64, move.to_64, move.promotion_type);
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        
        BitboardPosition new_pos = pos;
        if (new_pos.make_move(simple_move)) {
            string move_str = square_to_string(move.from_64) + square_to_string(move.to_64);
            
            string current_path = move_prefix + (move_prefix.empty() ? "" : "-") + move_str;
            uint64_t child_nodes = debug_perft_original(new_pos, depth - 1, current_path, level + 1);
            total_nodes += child_nodes;
            
            if (depth == 3 && level == 0) {
                cout << "Original depth 3 from " << move_str << ": " << child_nodes << " nodes" << endl;
            }
        }
    }
    
    return total_nodes;
}

// Manual perft that shows every step for optimized method
uint64_t debug_perft_optimized(BitboardPosition& pos, int depth, const string& move_prefix = "", int level = 0) {
    if (depth == 0) return 1;
    
    if (depth == 1) {
        BitboardMoveList moves;
        BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);
        return moves.moves.size();
    }
    
    BitboardMoveList moves;
    BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);
    
    uint64_t total_nodes = 0;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move(move.from_64, move.to_64, move.promotion_type);
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        
        BitboardPosition new_pos = pos;
        if (new_pos.make_move(simple_move)) {
            string move_str = square_to_string(move.from_64) + square_to_string(move.to_64);
            
            string current_path = move_prefix + (move_prefix.empty() ? "" : "-") + move_str;
            uint64_t child_nodes = debug_perft_optimized(new_pos, depth - 1, current_path, level + 1);
            total_nodes += child_nodes;
            
            if (depth == 3 && level == 0) {
                cout << "Optimized depth 3 from " << move_str << ": " << child_nodes << " nodes" << endl;
            }
        }
    }
    
    return total_nodes;
}

int main() {
    cout << "=== Manual Perft Counting Debug ===" << endl;
    
    // Initialize engine
    Huginn::init();
    if (!Huginn::is_initialized()) {
        cout << "ERROR: Failed to initialize engine" << endl;
        return 1;
    }
    
    // Starting position
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "Failed to parse starting position" << endl;
        return 1;
    }
    
    cout << "Checking move count at depth 1:" << endl;
    
    BitboardMoveList orig_moves, opt_moves;
    BitboardMoveGen::generate_legal_moves(pos, orig_moves);
    BitboardPerftOptimized::generate_legal_moves_fast(pos, opt_moves);
    
    cout << "Original method: " << orig_moves.moves.size() << " moves" << endl;
    cout << "Optimized method: " << opt_moves.moves.size() << " moves" << endl;
    
    if (orig_moves.moves.size() != opt_moves.moves.size()) {
        cout << "ERROR: Different move counts at depth 1!" << endl;
        return 1;
    }
    
    cout << "\nRunning manual depth 3 perft with detailed output..." << endl;
    
    BitboardPosition pos_copy1 = pos;
    cout << "\n=== Original Method Breakdown ===" << endl;
    uint64_t orig_total = debug_perft_original(pos_copy1, 3);
    cout << "Original total: " << orig_total << endl;
    
    BitboardPosition pos_copy2 = pos;
    cout << "\n=== Optimized Method Breakdown ===" << endl;
    uint64_t opt_total = debug_perft_optimized(pos_copy2, 3);
    cout << "Optimized total: " << opt_total << endl;
    
    cout << "\nDifference: " << (int64_t)opt_total - (int64_t)orig_total << endl;
    
    return 0;
}