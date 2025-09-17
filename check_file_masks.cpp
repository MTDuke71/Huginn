/**
 * @file check_file_masks.cpp
 * @brief Check what the file mask constants actually represent
 */

#include <iostream>
#include <iomanip>

using namespace std;

void print_bitboard(uint64_t bb, const string& name) {
    cout << name << ":" << endl;
    for (int rank = 7; rank >= 0; rank--) {
        cout << (rank + 1) << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if (bb & (1ULL << square)) {
                cout << "1 ";
            } else {
                cout << ". ";
            }
        }
        cout << endl;
    }
    cout << "  a b c d e f g h" << endl << endl;
}

int main() {
    uint64_t A_FILE = 0x0101010101010101ULL;
    uint64_t H_FILE = 0x8080808080808080ULL;
    
    print_bitboard(A_FILE, "A_FILE (0x0101010101010101ULL)");
    print_bitboard(H_FILE, "H_FILE (0x8080808080808080ULL)");
    print_bitboard(~A_FILE, "~A_FILE (mask to remove A-file)");
    print_bitboard(~H_FILE, "~H_FILE (mask to remove H-file)");
    
    return 0;
}