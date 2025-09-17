/**
 * @file debug_kiwipete_specific.cpp
 * @brief Debug the Kiwipete position specifically to find the -43 move discrepancy
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
    return string(1, 'a' + file) + to_string(rank + 1);
}

string move_to_string(int from, int to) {
    return square_to_string(from) + square_to_string(to);
}

int main() {
    cout << "=== Debugging Kiwipete Position Specifically ===" << endl;
    
    // Initialize
    init_attack_tables();
    
    // Kiwipete position: r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1
    string fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    BitboardPosition pos;
    pos.set_from_fen(fen);
    
    cout << "FEN: " << fen << endl;
    cout << "This is the famous Kiwipete position with complex tactics" << endl;
    
    // Generate moves with both methods
    MoveList original_moves;
    generate_moves_bitboard_pure(pos, original_moves);
    
    BitboardMoveList optimized_moves;
    generate_moves_optimized(pos, optimized_moves);
    
    cout << "\nOriginal method: " << original_moves.moves.size() << " moves" << endl;
    cout << "Optimized method: " << optimized_moves.moves.size() << " moves" << endl;
    cout << "Difference: " << (int)optimized_moves.moves.size() - (int)original_moves.moves.size() << endl;
    
    // Convert to sets for comparison
    set<string> original_set, optimized_set;
    
    for (const auto& move : original_moves.moves) {
        string move_str = move_to_string(move.from_square, move.to_square);
        if (move.is_promotion) {
            // Add promotion piece indicator
            char promo_char = 'q'; // Default to queen, would need to check actual promotion type
            move_str += promo_char;
        }
        original_set.insert(move_str);
    }
    
    for (const auto& move : optimized_moves.moves) {
        string move_str = move_to_string(move.from_square, move.to_square);
        if (move.is_promotion) {
            char promo_char = 'q'; // Simplified
            move_str += promo_char;
        }
        optimized_set.insert(move_str);
    }
    
    // Find missing moves (in original but not optimized)
    cout << "\nMoves in ORIGINAL but NOT in optimized:" << endl;
    int missing_count = 0;
    for (const string& move : original_set) {
        if (optimized_set.find(move) == optimized_set.end()) {
            cout << "  MISSING: " << move << endl;
            missing_count++;
        }
    }
    
    // Find extra moves (in optimized but not original)
    cout << "\nMoves in OPTIMIZED but NOT in original:" << endl;
    int extra_count = 0;
    for (const string& move : optimized_set) {
        if (original_set.find(move) == original_set.end()) {
            cout << "  EXTRA: " << move << endl;
            extra_count++;
        }
    }
    
    cout << "\nSummary:" << endl;
    cout << "Missing moves: " << missing_count << endl;
    cout << "Extra moves: " << extra_count << endl;
    cout << "Net difference: " << extra_count - missing_count << " (should be -43)" << endl;
    
    return 0;
}