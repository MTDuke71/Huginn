#include <iostream>
#include <iomanip>

void print_bitboard(uint64_t bb, const std::string& name) {
    std::cout << "\n" << name << ":\n";
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << " ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            std::cout << ((bb & (1ULL << square)) ? "1 " : ". ");
        }
        std::cout << "\n";
    }
    std::cout << "  a b c d e f g h\n";
    std::cout << "Hex: 0x" << std::hex << bb << std::dec << "\n";
}

int main() {
    std::cout << "=== MASK VERIFICATION ===\n";
    
    const uint64_t a_file_mask = 0x7F7F7F7F7F7F7F7FULL;
    const uint64_t h_file_mask = 0xFEFEFEFEFEFEFEFEULL;
    
    print_bitboard(a_file_mask, "A-file mask (should exclude A-file)");
    print_bitboard(h_file_mask, "H-file mask (should exclude H-file)");
    
    const uint64_t a_file = 0x0101010101010101ULL;
    const uint64_t h_file = 0x8080808080808080ULL;
    
    print_bitboard(a_file, "Actual A-file");
    print_bitboard(h_file, "Actual H-file");
    
    // Check bit 23 (h3)
    std::cout << "\nBit 23 (h3) analysis:\n";
    std::cout << "Bit 23 in A-file mask: " << ((a_file_mask & (1ULL << 23)) ? "YES" : "NO") << "\n";
    std::cout << "Bit 23 in H-file mask: " << ((h_file_mask & (1ULL << 23)) ? "YES" : "NO") << "\n";
    std::cout << "Bit 23 in actual A-file: " << ((a_file & (1ULL << 23)) ? "YES" : "NO") << "\n";
    std::cout << "Bit 23 in actual H-file: " << ((h_file & (1ULL << 23)) ? "YES" : "NO") << "\n";
    
    // Square 23 calculation
    const int rank = 23 / 8;
    const int file = 23 % 8;
    std::cout << "Square 23: rank=" << rank << ", file=" << file << " (" << char('a' + file) << (rank + 1) << ")\n";
    
    return 0;
}