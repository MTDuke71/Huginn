/**
 * @file debug_perft_depth2.cpp
 * @brief Debug perft discrepancies at depth 2
 */

#include <iostream>
#include <iomanip>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

void debug_depth2_starting_position() {
    cout << "=== Debugging Depth 2 - Starting Position ===" << endl;
    
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "Failed to parse starting position" << endl;
        return;
    }
    
    // Get all legal moves from original and optimized
    BitboardMoveList original_moves, optimized_moves;
    
    BitboardPosition pos_copy = pos;
    BitboardMoveGen::generate_legal_moves(pos_copy, original_moves);
    BitboardPerftOptimized::generate_legal_moves_fast(pos, optimized_moves);
    
    cout << "Depth 1: Original " << original_moves.moves.size() << ", Optimized " << optimized_moves.moves.size() << endl;
    
    uint64_t original_total = 0;
    uint64_t optimized_total = 0;
    
    cout << "\nMove-by-move depth 2 analysis:" << endl;
    cout << "Move                Original  Optimized" << endl;
    cout << "----------------------------------------" << endl;
    
    // Process each move from original method
    for (size_t i = 0; i < min(original_moves.moves.size(), optimized_moves.moves.size()); i++) {
        const auto& orig_move = original_moves.moves[i];
        const auto& opt_move = optimized_moves.moves[i];
        
        // Convert to SimpleBitboardMove
        SimpleBitboardMove simple_orig(orig_move.from_64, orig_move.to_64, orig_move.promotion_type);
        simple_orig.is_capture = orig_move.is_capture;
        simple_orig.is_ep_capture = orig_move.is_ep_capture;
        simple_orig.is_castling = orig_move.is_castling;
        simple_orig.is_promotion = orig_move.is_promotion;
        
        SimpleBitboardMove simple_opt(opt_move.from_64, opt_move.to_64, opt_move.promotion_type);
        simple_opt.is_capture = opt_move.is_capture;
        simple_opt.is_ep_capture = opt_move.is_ep_capture;
        simple_opt.is_castling = opt_move.is_castling;
        simple_opt.is_promotion = opt_move.is_promotion;
        
        // Make move and count responses
        BitboardPosition pos_orig = pos;
        BitboardPosition pos_opt = pos;
        
        uint64_t orig_responses = 0;
        uint64_t opt_responses = 0;
        
        if (pos_orig.make_move(simple_orig)) {
            BitboardMoveList responses;
            BitboardMoveGen::generate_legal_moves(pos_orig, responses);
            orig_responses = responses.moves.size();
            original_total += orig_responses;
        }
        
        if (pos_opt.make_move(simple_opt)) {
            BitboardMoveList responses;
            BitboardPerftOptimized::generate_legal_moves_fast(pos_opt, responses);
            opt_responses = responses.moves.size();
            optimized_total += opt_responses;
        }
        
        // Convert move to algebraic notation
        string move_str = "";
        move_str += char('a' + (orig_move.from_64 % 8));
        move_str += char('1' + (orig_move.from_64 / 8));
        move_str += char('a' + (orig_move.to_64 % 8));
        move_str += char('1' + (orig_move.to_64 / 8));
        
        cout << setw(16) << move_str << setw(8) << orig_responses << setw(10) << opt_responses;
        if (orig_responses != opt_responses) {
            cout << " <-- MISMATCH";
        }
        cout << endl;
    }
    
    cout << "----------------------------------------" << endl;
    cout << "Total depth 2:      " << setw(8) << original_total << setw(10) << optimized_total << endl;
    cout << "Expected: 400" << endl;
}

int main() {
    cout << "Debug Perft Depth 2 Discrepancies" << endl;
    cout << "==================================" << endl;
    
    try {
        Huginn::init();
        if (!Huginn::is_initialized()) {
            cout << "ERROR: Failed to initialize engine" << endl;
            return 1;
        }
        
        debug_depth2_starting_position();
        
    } catch (const exception& e) {
        cout << "Exception: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}