/**
 * @file debug_worst_offenders.cpp
 * @brief Deep analysis of e2e3, e2e4, h2h4 moves that show largest discrepancies
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
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

void analyze_position_detailed(const BitboardPosition& pos, const string& position_name, int depth) {
    if (depth == 0) return;
    
    cout << "\n=== Analyzing " << position_name << " (depth " << depth << ") ===" << endl;
    cout << "FEN: " << pos.to_fen() << endl;
    
    // Generate moves with both methods
    BitboardMoveList orig_moves, opt_moves;
    BitboardMoveGen::generate_legal_moves(const_cast<BitboardPosition&>(pos), orig_moves);
    BitboardPerftOptimized::generate_legal_moves_fast(pos, opt_moves);
    
    cout << "Move counts: Original " << orig_moves.moves.size() << ", Optimized " << opt_moves.moves.size() << endl;
    
    if (orig_moves.moves.size() != opt_moves.moves.size()) {
        cout << "*** MOVE COUNT MISMATCH! ***" << endl;
    }
    
    // Convert to comparable format
    map<string, bool> orig_move_set, opt_move_set;
    
    cout << "\nOriginal moves:" << endl;
    for (const auto& move : orig_moves.moves) {
        string move_str = move_to_string(move);
        orig_move_set[move_str] = true;
        cout << "  " << move_str << endl;
    }
    
    cout << "\nOptimized moves:" << endl;
    for (const auto& move : opt_moves.moves) {
        string move_str = move_to_string(move);
        opt_move_set[move_str] = true;
        cout << "  " << move_str << endl;
    }
    
    // Find differences
    vector<string> missing_in_opt, extra_in_opt;
    
    for (const auto& pair : orig_move_set) {
        if (opt_move_set.find(pair.first) == opt_move_set.end()) {
            missing_in_opt.push_back(pair.first);
        }
    }
    
    for (const auto& pair : opt_move_set) {
        if (orig_move_set.find(pair.first) == orig_move_set.end()) {
            extra_in_opt.push_back(pair.first);
        }
    }
    
    if (!missing_in_opt.empty()) {
        cout << "\nMissing in optimized:" << endl;
        for (const string& move : missing_in_opt) {
            cout << "  " << move << " *** MISSING ***" << endl;
        }
    }
    
    if (!extra_in_opt.empty()) {
        cout << "\nExtra in optimized:" << endl;
        for (const string& move : extra_in_opt) {
            cout << "  " << move << " *** EXTRA ***" << endl;
        }
    }
    
    if (missing_in_opt.empty() && extra_in_opt.empty()) {
        cout << "\nMove lists are identical - discrepancy is in deeper recursion" << endl;
    }
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

void analyze_move_recursively(const BitboardPosition& start_pos, const string& move_name, 
                              SimpleBitboardMove move, int max_depth) {
    cout << "\n" << string(60, '=') << endl;
    cout << "ANALYZING MOVE: " << move_name << endl;
    cout << string(60, '=') << endl;
    
    BitboardPosition pos1 = start_pos;
    auto undo_info = pos1.make_move_with_undo(move);
    
    for (int depth = 1; depth <= max_depth; depth++) {
        uint64_t orig = perft_original(pos1, depth);
        uint64_t opt = perft_optimized_recursive(pos1, depth);
        int diff = (int)opt - (int)orig;
        
        cout << "Depth " << depth << ": Original " << orig << ", Optimized " << opt;
        if (diff != 0) {
            cout << " (diff: " << diff << ") ***";
        }
        cout << endl;
        
        // If we find a discrepancy at depth 1, analyze the position in detail
        if (depth == 1 && diff != 0) {
            analyze_position_detailed(pos1, move_name + " position", 1);
        }
        
        // For depth 3 discrepancies, let's also analyze in detail to find the issue
        if (depth == 3 && diff != 0) {
            cout << "\n>>> DEPTH 3 DISCREPANCY FOUND! Analyzing position in detail..." << endl;
            analyze_position_detailed(pos1, move_name + " position at depth 3 issue", 2);
        }
        
        // If differences are too large or we found the issue, stop
        if (abs(diff) > 100 || (depth == 1 && diff != 0)) {
            break;
        }
    }
    
    pos1.unmake_move(move, undo_info);
}

int main() {
    cout << "=== Analyzing Worst Depth 4 Offenders ===" << endl;
    
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
    
    cout << "Starting Position: " << pos.to_fen() << endl;
    
    // Analyze the worst offenders
    analyze_move_recursively(pos, "e2e3", SimpleBitboardMove(12, 20), 3);  // e2=12, e3=20
    analyze_move_recursively(pos, "e2e4", SimpleBitboardMove(12, 28), 3);  // e2=12, e4=28  
    analyze_move_recursively(pos, "h2h4", SimpleBitboardMove(15, 31), 3);  // h2=15, h4=31
    
    return 0;
}