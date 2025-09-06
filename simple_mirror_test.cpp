// Simple mirror test to debug piece-by-piece
#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/init.hpp"
#include <iostream>
#include "src/board120.hpp"

using namespace Huginn;

void test_single_piece_mirror() {
    std::cout << "=== Testing Single Piece Mirror ===" << std::endl;
    
    // Test black knight on c6 (sq120 = 72)
    int sq120_c6 = 72;  // c6 in 120-square system
    std::cout << "Original square c6 (120): " << sq120_c6 << std::endl;
    
    int sq64_c6 = MAILBOX_MAPS.to64[sq120_c6];
    std::cout << "c6 in 64-square: " << sq64_c6 << std::endl;
    
    // mirror64 array should map this to c3
    extern const int mirror64[64];
    int mirrored_sq64 = sq64_c6 >= 0 ? mirror64[sq64_c6] : -1;
    std::cout << "Mirrored 64-square: " << mirrored_sq64 << std::endl;
    
    int mirrored_sq120 = mirrored_sq64 >= 0 ? MAILBOX_MAPS.to120[mirrored_sq64] : -1;
    std::cout << "Mirrored square c3 (120): " << mirrored_sq120 << std::endl;
    
    // Let's see what square c3 should be
    int sq120_c3 = 42;  // c3 in 120-square system
    std::cout << "Expected c3 (120): " << sq120_c3 << std::endl;
    
    if (mirrored_sq120 == sq120_c3) {
        std::cout << "✓ Square mirroring works correctly!" << std::endl;
    } else {
        std::cout << "✗ Square mirroring is WRONG!" << std::endl;
    }
}

int main() {
    init();
    test_single_piece_mirror();
    return 0;
}
