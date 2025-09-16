#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;
using namespace std;

// Convert 64-square to algebraic notation
string square64_to_algebraic(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return string(1, 'a' + file) + string(1, '1' + rank);
}

int main() {
    cout << "=== Manual Square Calculation Check ===" << endl;
    
    // Verify square calculations
    cout << "c5 = file 2, rank 4 = " << (2 + 4*8) << " -> " << square64_to_algebraic(2 + 4*8) << endl;
    cout << "d3 = file 3, rank 2 = " << (3 + 2*8) << " -> " << square64_to_algebraic(3 + 2*8) << endl;
    cout << "d4 = file 3, rank 3 = " << (3 + 3*8) << " -> " << square64_to_algebraic(3 + 3*8) << endl;
    
    cout << "\nFor Black pawn en passant capture from c5 to d3:" << endl;
    cout << "From square: " << (2 + 4*8) << " (c5)" << endl;
    cout << "To square: " << (3 + 2*8) << " (d3)" << endl;
    cout << "Difference: " << (2 + 4*8) - (3 + 2*8) << endl;
    
    cout << "\nExpected calculation for finding c5 pawn:" << endl;
    cout << "ep_square + 8 + 1 = " << (3 + 2*8) << " + 8 + 1 = " << ((3 + 2*8) + 8 + 1) << endl;
    cout << "That square is: " << square64_to_algebraic((3 + 2*8) + 8 + 1) << endl;
    
    cout << "\nExpected calculation for finding c5 pawn (alternative):" << endl;
    cout << "ep_square + 8 - 1 = " << (3 + 2*8) << " + 8 - 1 = " << ((3 + 2*8) + 8 - 1) << endl;
    cout << "That square is: " << square64_to_algebraic((3 + 2*8) + 8 - 1) << endl;
    
    cout << "\nFor the en passant scenario, we should check the 4th rank for Black pawns:" << endl;
    int ep_square = 3 + 2*8; // d3
    cout << "ep_square = " << ep_square << " (d3)" << endl;
    
    // Check squares on rank 4 (one rank up from d3) for Black pawns
    int rank_4_start = 32; // rank 4 starts at square 32 (a4)
    for (int file = 0; file < 8; file++) {
        int square = rank_4_start + file;
        cout << "Rank 4, file " << file << " = square " << square << " (" << square64_to_algebraic(square) << ")" << endl;
        
        // Check if this square can capture en passant to d3
        if (abs((square % 8) - (ep_square % 8)) == 1) { // adjacent files
            cout << "  -> Adjacent to d3, can capture en passant!" << endl;
        }
    }
    
    return 0;
}