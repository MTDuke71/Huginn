/**
 * @file debug_endgame_position.cpp
 * @brief Debug specific moves in the complex endgame position
 */

#include <iostream>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

void debug_specific_moves() {
    cout << "=== Debugging Complex Endgame Position ===" << endl;
    const char* fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    cout << "FEN: " << fen << endl;
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        cout << "ERROR: Failed to parse FEN" << endl;
        return;
    }
    
    int king_square = 32; // a5
    
    // Check pin detection
    cout << "\n=== Pin Detection ===" << endl;
    uint64_t pinned = BitboardPerftOptimized::find_pinned_pieces(pos, king_square);
    cout << "Pinned pieces bitboard: 0x" << hex << pinned << dec << endl;
    
    // Check if b5 pawn (square 33) is pinned
    bool b5_pinned = (pinned & (1ULL << 33)) != 0;
    cout << "Is b5 pawn pinned? " << (b5_pinned ? "YES" : "NO") << endl;
    
    // Check specific moves
    cout << "\n=== Checking a5b6 King Move ===" << endl;
    int target_square = 41; // b6
    
    // Check if b6 is attacked by black
    bool b6_attacked = BitboardPerftOptimized::is_square_attacked_fast(pos, target_square, Color::Black);
    cout << "Is b6 attacked by Black? " << (b6_attacked ? "YES" : "NO") << endl;
    
    // Check if the king move would be legal
    bool legal = !BitboardPerftOptimized::would_be_attacked_after_king_move(pos, target_square, Color::Black, king_square);
    cout << "Is a5b6 legal? " << (legal ? "YES" : "NO") << endl;
    
    cout << "\n=== Checking b5b6 Pawn Move ===" << endl;
    // Check if b6 is occupied
    bool b6_occupied = pos.is_square_occupied(target_square);
    cout << "Is b6 occupied? " << (b6_occupied ? "YES" : "NO") << endl;
    
    // Show piece positions relevant to pin
    cout << "\n=== Pin Analysis ===" << endl;
    cout << "White King a5 (square 32): " << (pos.is_square_occupied(32) ? "Present" : "Missing") << endl;
    cout << "White Pawn b5 (square 33): " << (pos.is_square_occupied(33) ? "Present" : "Missing") << endl;
    cout << "Black Rook h5 (square 39): " << (pos.is_square_occupied(39) ? "Present" : "Missing") << endl;
    
    // Check what's on rank 5 (squares 32-39)
    cout << "Rank 5 occupancy: ";
    for (int sq = 32; sq < 40; sq++) {
        if (pos.is_square_occupied(sq)) {
            char file = 'a' + (sq % 8);
            cout << file << "5 ";
        }
    }
    cout << endl;
}

int main() {
    try {
        Huginn::init();
        if (!Huginn::is_initialized()) {
            cout << "ERROR: Failed to initialize engine" << endl;
            return 1;
        }
        
        debug_specific_moves();
        
    } catch (const exception& e) {
        cout << "Exception caught: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}