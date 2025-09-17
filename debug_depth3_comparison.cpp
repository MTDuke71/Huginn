#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iomanip>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

int main() {
    cout << "=== Simple Depth 3 Debugging ===" << endl;
    
    // Initialize
    init_pawn_lookup_tables();
    initialize_bitboard_attack_tables();
    
    // Starting position
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "Failed to parse starting position" << endl;
        return 1;
    }
    
    cout << "Testing starting position with depth 3..." << endl;
    
    // Run both methods
    uint64_t original_count = perft_recursive_original(pos, 3);
    uint64_t optimized_count = perft_recursive_optimized(pos, 3);
    
    cout << "Original method:   " << original_count << endl;
    cout << "Optimized method:  " << optimized_count << endl;
    cout << "Difference: " << (int64_t)optimized_count - (int64_t)original_count << endl;
    
    if (original_count != optimized_count) {
        cout << "\nLet's check depth 2 moves individually..." << endl;
        
        // Get depth 1 moves from starting position
        MovemaskT moves = pos.get_legal_moves();
        int move_count = 0;
        
        while (moves) {
            SimpleBitboardMove move = pos.extract_next_move(&moves);
            move_count++;
            
            string move_str = pos.move_to_string(move);
            
            // Apply move and check depth 2
            BitboardPosition temp_pos = pos;
            temp_pos.make_move(move);
            
            uint64_t orig_depth2 = perft_recursive_original(temp_pos, 2);
            uint64_t opt_depth2 = perft_recursive_optimized(temp_pos, 2);
            
            if (orig_depth2 != opt_depth2) {
                cout << "DIFFERENCE at depth 2 after " << move_str 
                     << ": Original=" << orig_depth2 
                     << ", Optimized=" << opt_depth2 
                     << ", Diff=" << (int64_t)opt_depth2 - (int64_t)orig_depth2 << endl;
            }
        }
        
        cout << "Checked " << move_count << " moves at depth 1" << endl;
    }
    
    return 0;
}