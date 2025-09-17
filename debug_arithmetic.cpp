#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== BITBOARD ARITHMETIC CHECK ===\n";
    
    const uint64_t black_pieces = 0x917d730002800000ULL;
    const uint64_t h3_bit = 0x800000ULL; // bit 23
    const uint64_t g2_shifted = 0x800000ULL; // g2 << 9
    const uint64_t a_file_mask = 0x7F7F7F7F7F7F7F7FULL;
    
    std::cout << "black_pieces:     0x" << std::hex << black_pieces << std::dec << "\n";
    std::cout << "h3_bit:           0x" << std::hex << h3_bit << std::dec << "\n";
    std::cout << "g2_shifted:       0x" << std::hex << g2_shifted << std::dec << "\n";
    std::cout << "a_file_mask:      0x" << std::hex << a_file_mask << std::dec << "\n";
    
    const bool h3_in_black = (black_pieces & h3_bit) != 0;
    std::cout << "h3 in black_pieces: " << (h3_in_black ? "YES" : "NO") << "\n";
    
    const uint64_t intersection1 = g2_shifted & black_pieces;
    std::cout << "g2_shifted & black_pieces: 0x" << std::hex << intersection1 << std::dec << "\n";
    
    const uint64_t intersection2 = intersection1 & a_file_mask;
    std::cout << "final result: 0x" << std::hex << intersection2 << std::dec << "\n";
    
    const uint64_t g2_bit = 1ULL << 14;
    const bool g2_passes_mask = (g2_bit & a_file_mask) != 0;
    std::cout << "g2 passes A-file mask: " << (g2_passes_mask ? "YES" : "NO") << "\n";
    
    return 0;
}