#include "src/bitboard_position.hpp"
#include "src/bitboard_attacks.hpp"
#include "src/bitboard.hpp"
#include <iostream>
#include <iomanip>
#include <intrin.h>

// Test attack table initialization
int main() {
    std::cout << "=== Attack Table Initialization Test ===\n\n";

    // Initialize attack tables
    init_bitboard_attacks();

    std::cout << "Knight attacks from e4 (28):\n";
    uint64_t knight_attacks_e4 = knight_attacks[28];
    std::cout << "Bitboard: " << std::hex << (unsigned long long)knight_attacks_e4 << std::dec << "\n";
    std::cout << "Number of attacked squares: " << __popcnt64(knight_attacks_e4) << "\n";

    std::cout << "\nKing attacks from e4 (28):\n";
    uint64_t king_attacks_e4 = king_attacks[28];
    std::cout << "Bitboard: " << std::hex << (unsigned long long)king_attacks_e4 << std::dec << "\n";
    std::cout << "Number of attacked squares: " << __popcnt64(king_attacks_e4) << "\n";

    std::cout << "\nWhite pawn attacks from e4 (28):\n";
    uint64_t white_pawn_attacks_e4 = white_pawn_attacks[28];
    std::cout << "Bitboard: " << std::hex << (unsigned long long)white_pawn_attacks_e4 << std::dec << "\n";
    std::cout << "Number of attacked squares: " << __popcnt64(white_pawn_attacks_e4) << "\n";

    // Test bishop and rook attacks
    std::cout << "\nBishop attacks from e4 (28) with empty board:\n";
    uint64_t bishop_attacks_e4 = bishop_attacks(28, 0ULL);
    std::cout << "Bitboard: " << std::hex << (unsigned long long)bishop_attacks_e4 << std::dec << "\n";
    std::cout << "Number of attacked squares: " << __popcnt64(bishop_attacks_e4) << "\n";

    std::cout << "\nRook attacks from e4 (28) with empty board:\n";
    uint64_t rook_attacks_e4 = rook_attacks(28, 0ULL);
    std::cout << "Bitboard: " << std::hex << (unsigned long long)rook_attacks_e4 << std::dec << "\n";
    std::cout << "Number of attacked squares: " << __popcnt64(rook_attacks_e4) << "\n";

    // Test with some occupied squares
    uint64_t occupied = (1ULL << 21) | (1ULL << 37); // f3 and f5 occupied
    std::cout << "\nBishop attacks from e4 (28) with f3,f5 occupied:\n";
    uint64_t bishop_attacks_e4_occupied = bishop_attacks(28, occupied);
    std::cout << "Bitboard: " << std::hex << (unsigned long long)bishop_attacks_e4_occupied << std::dec << "\n";
    std::cout << "Number of attacked squares: " << __popcnt64(bishop_attacks_e4_occupied) << "\n";

    return 0;
}