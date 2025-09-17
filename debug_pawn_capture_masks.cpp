/**
 * @file debug_pawn_capture_masks.cpp
 * @brief Debug pawn capture mask calculations to identify wrap-around issues
 */

#include <iostream>
#include <iomanip>
#include <bitset>

using namespace std;

void print_bitboard(uint64_t bb, const string& name) {
    cout << name << ":" << endl;
    for (int rank = 7; rank >= 0; rank--) {
        cout << (rank + 1) << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            cout << ((bb >> square) & 1) << " ";
        }
        cout << endl;
    }
    cout << "  a b c d e f g h" << endl << endl;
}

int main() {
    cout << "=== Pawn Capture Mask Analysis ===" << endl;
    
    // File masks
    uint64_t FILE_A = 0x0101010101010101ULL;
    uint64_t FILE_H = 0x8080808080808080ULL;
    
    cout << "FILE_A = 0x" << hex << FILE_A << dec << endl;
    cout << "FILE_H = 0x" << hex << FILE_H << dec << endl;
    
    print_bitboard(FILE_A, "FILE_A");
    print_bitboard(FILE_H, "FILE_H");
    
    // Test case: White pawns on edge files
    uint64_t test_pawns_a = 0x0100ULL; // a2
    uint64_t test_pawns_h = 0x8000ULL; // h2
    
    cout << "Test pawn on a2:" << endl;
    print_bitboard(test_pawns_a, "Pawn on a2");
    
    cout << "Test pawn on h2:" << endl;
    print_bitboard(test_pawns_h, "Pawn on h2");
    
    // Current white pawn capture logic
    cout << "=== Current White Pawn Capture Logic ===" << endl;
    
    // Left captures (northwest)
    uint64_t left_mask = ~FILE_H; // Not H-file
    uint64_t left_captures_a = ((test_pawns_a & left_mask) << 7);
    uint64_t left_captures_h = ((test_pawns_h & left_mask) << 7);
    
    cout << "Left captures from a2 (should be valid to b3):" << endl;
    print_bitboard(left_captures_a, "a2 left capture");
    
    cout << "Left captures from h2 (should be blocked):" << endl;
    print_bitboard(left_captures_h, "h2 left capture");
    
    // Right captures (northeast) 
    uint64_t right_mask = ~FILE_A; // Not A-file
    uint64_t right_captures_a = ((test_pawns_a & right_mask) << 9);
    uint64_t right_captures_h = ((test_pawns_h & right_mask) << 9);
    
    cout << "Right captures from a2 (should be blocked):" << endl;
    print_bitboard(right_captures_a, "a2 right capture");
    
    cout << "Right captures from h2 (should be valid to g3):" << endl;
    print_bitboard(right_captures_h, "h2 right capture");
    
    // Test the problematic case: pawn near edge
    uint64_t test_pawn_g = 0x4000ULL; // g2
    uint64_t left_captures_g = ((test_pawn_g & left_mask) << 7);
    
    cout << "Left captures from g2 (should be valid to h3):" << endl;
    print_bitboard(left_captures_g, "g2 left capture");
    
    // Check for potential wrap-around
    if (left_captures_g & FILE_A) {
        cout << "*** WRAP-AROUND DETECTED: g2 left capture wraps to A-file! ***" << endl;
    }
    
    return 0;
}