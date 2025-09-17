/**
 * @file debug_depth2_recursive.cpp
 * @brief Analyze depth 2 recursion after problematic edge file moves
 * 
 * Since depth 1 moves after h2h3, a2a4, h2h4 are identical between methods,
 * but depth 2 perft shows discrepancies, this tool examines what happens
 * when we make each black response move and then generate white moves.
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <set>
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

// Analyze depth 2 recursion after a specific white move
void analyze_depth2_after(const string& white_move, int from_64, int to_64) {
    cout << "\n=== Analyzing depth 2 after " << white_move << " ===" << endl;
    
    // Set up initial position and make white move
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "Failed to parse starting position" << endl;
        return;
    }
    
    SimpleBitboardMove white_move_obj(from_64, to_64);
    if (!pos.make_move(white_move_obj)) {
        cout << "Failed to make white move " << white_move << endl;
        return;
    }
    
    cout << "Position after " << white_move << " (Black to move)" << endl;
    
    // Get black's responses using both methods
    BitboardMoveList black_moves_orig, black_moves_opt;
    BitboardMoveGen::generate_legal_moves(pos, black_moves_orig);
    BitboardPerftOptimized::generate_legal_moves_fast(pos, black_moves_opt);
    
    cout << "Black has " << black_moves_orig.moves.size() << " response moves" << endl;
    
    // For each black response, check white's depth 1 moves
    map<string, pair<int, int>> move_counts; // move -> (original_count, optimized_count)
    
    for (size_t i = 0; i < black_moves_orig.moves.size(); i++) {
        const auto& black_move = black_moves_orig.moves[i];
        string black_move_str = move_to_string(black_move);
        
        // Make black's move
        BitboardPosition after_black = pos;
        SimpleBitboardMove black_simple(black_move.from_64, black_move.to_64, black_move.promotion_type);
        black_simple.is_capture = black_move.is_capture;
        black_simple.is_ep_capture = black_move.is_ep_capture;
        black_simple.is_castling = black_move.is_castling;
        black_simple.is_promotion = black_move.is_promotion;
        
        if (!after_black.make_move(black_simple)) {
            cout << "Failed to make black move " << black_move_str << endl;
            continue;
        }
        
        // Now generate white's responses using both methods
        BitboardMoveList white_orig, white_opt;
        BitboardMoveGen::generate_legal_moves(after_black, white_orig);
        BitboardPerftOptimized::generate_legal_moves_fast(after_black, white_opt);
        
        move_counts[black_move_str] = {white_orig.moves.size(), white_opt.moves.size()};
        
        if (white_orig.moves.size() != white_opt.moves.size()) {
            cout << "*** DISCREPANCY after " << white_move << " " << black_move_str << " ***" << endl;
            cout << "Original: " << white_orig.moves.size() << " moves" << endl;
            cout << "Optimized: " << white_opt.moves.size() << " moves" << endl;
            cout << "Difference: " << (int)white_opt.moves.size() - (int)white_orig.moves.size() << endl;
            
            // Show which moves differ
            set<string> orig_set, opt_set;
            for (const auto& move : white_orig.moves) {
                orig_set.insert(move_to_string(move));
            }
            for (const auto& move : white_opt.moves) {
                opt_set.insert(move_to_string(move));
            }
            
            vector<string> only_orig, only_opt;
            set_difference(orig_set.begin(), orig_set.end(), opt_set.begin(), opt_set.end(), back_inserter(only_orig));
            set_difference(opt_set.begin(), opt_set.end(), orig_set.begin(), orig_set.end(), back_inserter(only_opt));
            
            if (!only_orig.empty()) {
                cout << "Only in original: ";
                for (const string& move : only_orig) cout << move << " ";
                cout << endl;
            }
            if (!only_opt.empty()) {
                cout << "Only in optimized: ";
                for (const string& move : only_opt) cout << move << " ";
                cout << endl;
            }
            
            return; // Found the discrepancy, no need to check more
        }
    }
    
    cout << "No discrepancies found in depth 2 after " << white_move << endl;
    cout << string(60, '-') << endl;
}

int main() {
    cout << "=== Depth 2 Recursive Analysis ===" << endl;
    
    // Initialize engine
    Huginn::init();
    if (!Huginn::is_initialized()) {
        cout << "ERROR: Failed to initialize engine" << endl;
        return 1;
    }
    
    cout << "Analyzing the three problematic moves at depth 2..." << endl;
    
    // Analyze each problematic move
    analyze_depth2_after("h2h3", 15, 23);  // h2 = 15, h3 = 23
    analyze_depth2_after("a2a4", 8, 24);   // a2 = 8, a4 = 24
    analyze_depth2_after("h2h4", 15, 31);  // h2 = 15, h4 = 31
    
    cout << "\n=== Analysis Complete ===" << endl;
    cout << "This should identify exactly where the edge file wrap-around" << endl;
    cout << "issue manifests in the recursive move generation." << endl;
    
    return 0;
}