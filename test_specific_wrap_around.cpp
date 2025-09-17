/**
 * @file test_specific_wrap_around.cpp
 * @brief Test the exact scenario where h3->a5 impossible move is generated
 */

#include <iostream>
#include <iomanip>
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

int main() {
    cout << "=== Testing Specific Wrap-Around Bug ===" << endl;
    
    // Initialize engine
    Huginn::init();
    if (!Huginn::is_initialized()) {
        cout << "ERROR: Failed to initialize engine" << endl;
        return 1;
    }
    
    // Recreate the exact position where h3->a5 is generated
    // 1. Start position
    // 2. h2h3 (white)
    // 3. a7a5 (black)
    // 4. Now generate white moves - one should be h3a5
    
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "Failed to parse starting position" << endl;
        return 1;
    }
    
    // Make h2h3
    SimpleBitboardMove h2h3(15, 23); // h2=15, h3=23
    if (!pos.make_move(h2h3)) {
        cout << "Failed to make h2h3" << endl;
        return 1;
    }
    cout << "After h2h3: " << pos.to_fen() << endl;
    
    // Make a7a5
    SimpleBitboardMove a7a5(48, 32); // a7=48, a5=32
    if (!pos.make_move(a7a5)) {
        cout << "Failed to make a7a5" << endl;
        return 1;
    }
    cout << "After h2h3 a7a5: " << pos.to_fen() << endl;
    
    // Now it's white to move - generate moves and look for h3a5
    cout << "\nGenerating white moves with optimized method..." << endl;
    BitboardMoveList moves;
    BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);
    
    cout << "Found " << moves.moves.size() << " moves:" << endl;
    
    bool found_h3a5 = false;
    for (const auto& move : moves.moves) {
        string move_str = move_to_string(move);
        cout << "  " << move_str;
        
        if (move_str == "h3a5") {
            cout << " *** IMPOSSIBLE WRAP-AROUND MOVE! ***";
            found_h3a5 = true;
            
            cout << "\n    from_64=" << move.from_64 << " (" << square_to_string(move.from_64) << ")";
            cout << "\n    to_64=" << move.to_64 << " (" << square_to_string(move.to_64) << ")";
            cout << "\n    is_capture=" << move.is_capture;
            cout << "\n    is_ep_capture=" << move.is_ep_capture;
        }
        cout << endl;
    }
    
    if (!found_h3a5) {
        cout << "\nNo h3a5 move found - wrap-around might be fixed or test setup wrong" << endl;
    }
    
    // Also test with original method for comparison
    cout << "\nGenerating white moves with original method..." << endl;
    BitboardMoveList orig_moves;
    BitboardMoveGen::generate_legal_moves(pos, orig_moves);
    
    cout << "Found " << orig_moves.moves.size() << " moves with original method" << endl;
    
    return 0;
}