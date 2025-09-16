#include "src/bitboard_position.hpp"
#include "src/bitboard_attacks.hpp"
#include "src/bitboard.hpp"
#include <iostream>
#include <iomanip>

// Test knight attack table values
int main() {
    std::cout << "=== Knight Attack Table Test ===\n\n";
    ensure_bitboard_attacks_initialized();


    // Test specific squares
    int test_squares[] = {18, 28}; // c3, e4

    for (int sq : test_squares) {
        std::cout << "Square " << sq << " knight attacks: " << std::hex << knight_attacks[sq] << std::dec << "\n";
        std::cout << "Number of attacks: " << __popcnt64(knight_attacks[sq]) << "\n";

        // List attacked squares
        std::cout << "Attacked squares: ";
        for (int i = 0; i < 64; i++) {
            if (knight_attacks[sq] & (1ULL << i)) {
                std::cout << i << " ";
            }
        }
        std::cout << "\n\n";
    }

    return 0;
}