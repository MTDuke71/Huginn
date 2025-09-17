/**
 * @file debug_depth3_moves.cpp
 * @brief Analyze specific depth 3 moves after e2e3 to find discrepancies
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

void analyze_depth2_after_move(const BitboardPosition& start_pos, const string& move_name, SimpleBitboardMove first_move) {
    cout << "\n" << string(60, '=') << endl;
    cout << "DEPTH 2 ANALYSIS AFTER: " << move_name << endl;
    cout << string(60, '=') << endl;
    
    // Make the first move
    BitboardPosition pos1 = start_pos;
    auto undo_info1 = pos1.make_move_with_undo(first_move);
    
    cout << "Position after " << move_name << ": " << pos1.to_fen() << endl;
    
    // Generate all possible second moves
    BitboardMoveList moves1;
    BitboardMoveGen::generate_legal_moves(pos1, moves1);
    
    cout << "\nMove-by-move depth 2 analysis from " << move_name << ":" << endl;
    cout << "Move       Original   Optimized   Diff" << endl;
    cout << "------     ---------  ---------   ----" << endl;
    
    uint64_t total_orig = 0, total_opt = 0;
    int moves_with_diff = 0;
    
    for (const auto& move : moves1.moves) {
        BitboardPosition pos2 = pos1;
        SimpleBitboardMove simple_move(move.from_64, move.to_64);
        auto undo_info2 = pos2.make_move_with_undo(simple_move);
        
        uint64_t orig_nodes = perft_original(pos2, 1);  // depth 1 from here = depth 3 total
        uint64_t opt_nodes = perft_optimized_recursive(pos2, 1);
        
        pos2.unmake_move(simple_move, undo_info2);
        
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
        
        // If this specific move shows a discrepancy, analyze it in detail
        if (diff != 0) {
            cout << "\n    >>> FOUND DISCREPANCY! Analyzing position after " 
                 << move_name << " " << move_to_string(move) << " <<<" << endl;
            
            // Check move generation at this specific position
            BitboardMoveList orig_moves, opt_moves;
            BitboardMoveGen::generate_legal_moves(pos2, orig_moves);
            BitboardPerftOptimized::generate_legal_moves_fast(pos2, opt_moves);
            
            cout << "    Position: " << pos2.to_fen() << endl;
            cout << "    Original moves: " << orig_moves.moves.size() << ", Optimized: " << opt_moves.moves.size() << endl;
            
            if (orig_moves.moves.size() != opt_moves.moves.size()) {
                cout << "    *** MOVE COUNT MISMATCH! ***" << endl;
                
                // Show the actual move differences
                set<string> orig_set, opt_set;
                for (const auto& m : orig_moves.moves) orig_set.insert(move_to_string(m));
                for (const auto& m : opt_moves.moves) opt_set.insert(move_to_string(m));
                
                for (const string& m : orig_set) {
                    if (opt_set.find(m) == opt_set.end()) {
                        cout << "    Missing in optimized: " << m << endl;
                    }
                }
                for (const string& m : opt_set) {
                    if (orig_set.find(m) == orig_set.end()) {
                        cout << "    Extra in optimized: " << m << endl;
                    }
                }
            }
            cout << endl;
        }
    }
    
    cout << "------     ---------  ---------   ----" << endl;
    cout << "Total:     " << setw(9) << total_orig << "  " << setw(9) << total_opt << "  " << setw(4) << ((int)total_opt - (int)total_orig) << endl;
    cout << "Moves with differences: " << moves_with_diff << " out of " << moves1.moves.size() << endl;
    
    pos1.unmake_move(first_move, undo_info1);
}

int main() {
    cout << "=== Depth 3 Move Analysis ===" << endl;
    
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
    
    // Focus on e2e3 which has -76 difference
    analyze_depth2_after_move(pos, "e2e3", SimpleBitboardMove(12, 20));  // e2=12, e3=20
    
    return 0;
}