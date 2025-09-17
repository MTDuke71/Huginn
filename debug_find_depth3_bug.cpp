/**
 * @file debug_find_depth3_bug.cpp
 * @brief Systematically find positions at depth 3 that have move generation differences
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <set>
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

struct PositionAnalysis {
    string move_sequence;
    string fen;
    int orig_moves;
    int opt_moves;
    int difference;
    vector<string> missing_moves;
    vector<string> extra_moves;
};

bool analyze_position_moves(const BitboardPosition& pos, const string& move_sequence, PositionAnalysis& analysis) {
    analysis.move_sequence = move_sequence;
    analysis.fen = pos.to_fen();
    
    // Generate moves with both methods
    BitboardMoveList orig_moves, opt_moves;
    BitboardMoveGen::generate_legal_moves(const_cast<BitboardPosition&>(pos), orig_moves);
    BitboardPerftOptimized::generate_legal_moves_fast(pos, opt_moves);
    
    analysis.orig_moves = orig_moves.moves.size();
    analysis.opt_moves = opt_moves.moves.size();
    analysis.difference = analysis.opt_moves - analysis.orig_moves;
    
    // Find move differences
    set<string> orig_set, opt_set;
    for (const auto& move : orig_moves.moves) {
        orig_set.insert(move_to_string(move));
    }
    for (const auto& move : opt_moves.moves) {
        opt_set.insert(move_to_string(move));
    }
    
    // Find missing and extra moves
    for (const string& move : orig_set) {
        if (opt_set.find(move) == opt_set.end()) {
            analysis.missing_moves.push_back(move);
        }
    }
    for (const string& move : opt_set) {
        if (orig_set.find(move) == orig_set.end()) {
            analysis.extra_moves.push_back(move);
        }
    }
    
    return analysis.difference != 0;
}

void explore_depth3_recursive(const BitboardPosition& pos, const string& move_sequence, int depth_remaining, vector<PositionAnalysis>& problems) {
    if (depth_remaining == 0) {
        // We've reached depth 3, analyze this position
        PositionAnalysis analysis;
        if (analyze_position_moves(pos, move_sequence, analysis)) {
            problems.push_back(analysis);
        }
        return;
    }
    
    // Generate moves and recurse
    BitboardMoveList moves;
    BitboardMoveGen::generate_legal_moves(const_cast<BitboardPosition&>(pos), moves);
    
    for (const auto& move : moves.moves) {
        BitboardPosition new_pos = pos;
        SimpleBitboardMove simple_move(move.from_64, move.to_64);
        if (new_pos.make_move(simple_move)) {
            string new_sequence = move_sequence + (move_sequence.empty() ? "" : " ") + move_to_string(move);
            explore_depth3_recursive(new_pos, new_sequence, depth_remaining - 1, problems);
        }
    }
}

int main() {
    cout << "=== Finding Depth 3 Problematic Positions ===" << endl;
    
    // Initialize engine
    Huginn::init();
    if (!Huginn::is_initialized()) {
        cout << "ERROR: Failed to initialize engine" << endl;
        return 1;
    }
    
    // Start from position after e2e3
    BitboardPosition start_pos;
    if (!start_pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1")) {
        cout << "Failed to parse starting position after e2e3" << endl;
        return 1;
    }
    
    cout << "Starting from position after e2e3: " << start_pos.to_fen() << endl;
    cout << "Exploring all positions at depth 3 to find move generation differences..." << endl << endl;
    
    vector<PositionAnalysis> problems;
    explore_depth3_recursive(start_pos, "", 3, problems);
    
    cout << "=== ANALYSIS RESULTS ===" << endl;
    cout << "Found " << problems.size() << " positions with move generation differences:" << endl << endl;
    
    if (problems.empty()) {
        cout << "No problematic positions found at depth 3!" << endl;
        cout << "This suggests the bug might be in the perft counting logic itself," << endl;
        cout << "not in the move generation." << endl;
    } else {
        for (size_t i = 0; i < problems.size() && i < 10; i++) {  // Show first 10
            const auto& prob = problems[i];
            cout << "Problem " << (i+1) << ":" << endl;
            cout << "  Move sequence: " << prob.move_sequence << endl;
            cout << "  FEN: " << prob.fen << endl;
            cout << "  Original moves: " << prob.orig_moves << endl;
            cout << "  Optimized moves: " << prob.opt_moves << endl;
            cout << "  Difference: " << prob.difference << endl;
            
            if (!prob.missing_moves.empty()) {
                cout << "  Missing in optimized: ";
                for (const string& move : prob.missing_moves) {
                    cout << move << " ";
                }
                cout << endl;
            }
            
            if (!prob.extra_moves.empty()) {
                cout << "  Extra in optimized: ";
                for (const string& move : prob.extra_moves) {
                    cout << move << " ";
                }
                cout << endl;
            }
            cout << endl;
        }
        
        if (problems.size() > 10) {
            cout << "... and " << (problems.size() - 10) << " more." << endl;
        }
    }
    
    return 0;
}