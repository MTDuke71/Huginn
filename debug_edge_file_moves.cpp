/**
 * @file debug_edge_file_moves.cpp
 * @brief Deep analysis of edge file moves that cause perft discrepancies
 * 
 * Investigates what happens at depth 2 after making the problematic moves:
 * - h2h3 (+1 node difference)
 * - a2a4 (-1 node difference)  
 * - h2h4 (+1 node difference)
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

// Helper function to convert square index to string
string square_to_string(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return string(1, 'a' + file) + string(1, '1' + rank);
}

// Convert move to string notation (simplified)
string move_to_string(const BitboardMoveList::BitboardMove& move) {
    return square_to_string(move.from_64) + square_to_string(move.to_64);
}

// Analyze moves after a specific initial move
void analyze_moves_after(const string& initial_move, int from_64, int to_64) {
    cout << "\n=== Analyzing moves after " << initial_move << " ===" << endl;
    
    // Set up initial position
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "Failed to parse starting position" << endl;
        return;
    }
    
    // Make the initial move
    SimpleBitboardMove initial(from_64, to_64);
    BitboardPosition after_move = pos;
    if (!after_move.make_move(initial)) {
        cout << "Failed to make initial move " << initial_move << endl;
        return;
    }
    
    cout << "Position after " << initial_move << " (Black to move):" << endl;
    cout << after_move.to_fen() << endl;
    
    // Generate moves using both methods
    BitboardMoveList original_moves, optimized_moves;
    BitboardMoveGen::generate_legal_moves(after_move, original_moves);
    BitboardPerftOptimized::generate_legal_moves_fast(after_move, optimized_moves);
    
    cout << "Original method: " << original_moves.moves.size() << " moves" << endl;
    cout << "Optimized method: " << optimized_moves.moves.size() << " moves" << endl;
    
    if (original_moves.moves.size() != optimized_moves.moves.size()) {
        cout << "*** MOVE COUNT DIFFERENCE FOUND! ***" << endl;
        cout << "Difference: " << (int)optimized_moves.moves.size() - (int)original_moves.moves.size() << endl;
    }
    
    // Convert to sets of move strings for comparison
    set<string> original_set, optimized_set;
    
    cout << "\nOriginal method moves:" << endl;
    for (const auto& move : original_moves.moves) {
        string move_str = move_to_string(move);
        original_set.insert(move_str);
        cout << "  " << move_str;
        if (move.is_capture) cout << " (capture)";
        if (move.is_ep_capture) cout << " (en passant)";
        if (move.is_castling) cout << " (castling)";
        cout << endl;
    }
    
    cout << "\nOptimized method moves:" << endl;
    for (const auto& move : optimized_moves.moves) {
        string move_str = move_to_string(move);
        optimized_set.insert(move_str);
        cout << "  " << move_str;
        if (move.is_capture) cout << " (capture)";
        if (move.is_ep_capture) cout << " (en passant)";
        if (move.is_castling) cout << " (castling)";
        cout << endl;
    }
    
    // Find differences
    vector<string> only_in_original, only_in_optimized;
    
    set_difference(original_set.begin(), original_set.end(),
                   optimized_set.begin(), optimized_set.end(),
                   back_inserter(only_in_original));
                   
    set_difference(optimized_set.begin(), optimized_set.end(),
                   original_set.begin(), original_set.end(),
                   back_inserter(only_in_optimized));
    
    if (!only_in_original.empty()) {
        cout << "\nMoves ONLY in original method:" << endl;
        for (const string& move_str : only_in_original) {
            cout << "  " << move_str << endl;
        }
    }
    
    if (!only_in_optimized.empty()) {
        cout << "\nMoves ONLY in optimized method:" << endl;
        for (const string& move_str : only_in_optimized) {
            cout << "  " << move_str << endl;
        }
    }
    
    if (only_in_original.empty() && only_in_optimized.empty()) {
        cout << "\nNo move differences found - moves are identical!" << endl;
        cout << "This suggests the discrepancy occurs at deeper levels." << endl;
    }
    
    cout << string(60, '-') << endl;
}

int main() {
    cout << "=== Deep Edge File Move Analysis ===" << endl;
    
    // Initialize engine
    Huginn::init();
    if (!Huginn::is_initialized()) {
        cout << "ERROR: Failed to initialize engine" << endl;
        return 1;
    }
    
    // Analyze the three problematic moves  
    analyze_moves_after("h2h3", 15, 23);  // h2 = 15, h3 = 23
    analyze_moves_after("a2a4", 8, 24);   // a2 = 8, a4 = 24
    analyze_moves_after("h2h4", 15, 31);  // h2 = 15, h4 = 31
    
    cout << "\n=== Analysis Complete ===" << endl;
    cout << "If no move differences found at depth 1 after these moves," << endl;
    cout << "the issue likely occurs in deeper recursive calls or" << endl;
    cout << "in how moves are applied/unapplied during recursion." << endl;
    
    return 0;
}