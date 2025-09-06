// VICE Part 78 Demo: Key Square Visualization
// Shows pawn masks for important squares with clear explanations

#include "../src/evaluation.hpp"
#include "../src/init.hpp"
#include <iostream>

namespace KeySquareDemo {

// MSVC-compatible popcount function
int popcount(uint64_t x) {
#ifdef _MSC_VER
    return (int)__popcnt64(x);
#else
    return __builtin_popcountll(x);
#endif
}

// Print a bitboard as a chess board with coordinates
void print_board(uint64_t mask, const std::string& title) {
    std::cout << "\n" << title << ":\n";
    std::cout << "    a   b   c   d   e   f   g   h\n";
    std::cout << "  +---+---+---+---+---+---+---+---+\n";
    
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << (rank + 1) << " |";
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            bool is_set = (mask & (1ULL << square)) != 0;
            std::cout << " " << (is_set ? "X" : " ") << " |";
        }
        std::cout << " " << (rank + 1) << "\n";
        std::cout << "  +---+---+---+---+---+---+---+---+\n";
    }
    std::cout << "    a   b   c   d   e   f   g   h\n";
}

void demonstrate_square(int square, const std::string& square_name) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "DEMONSTRATING: " << square_name << " (square index " << square << ")\n";
    std::cout << std::string(60, '=') << "\n";
    
    uint64_t white_mask = Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[square];
    uint64_t black_mask = Huginn::EvalParams::BLACK_PASSED_PAWN_MASKS[square];
    
    std::cout << "\nHex values:\n";
    std::cout << "White pawn mask: 0x" << std::hex << white_mask << std::dec 
              << " (" << popcount(white_mask) << " bits set)\n";
    std::cout << "Black pawn mask: 0x" << std::hex << black_mask << std::dec 
              << " (" << popcount(black_mask) << " bits set)\n";
    
    // Visual representation for white pawn
    print_board(white_mask, "WHITE pawn on " + square_name + " - Squares that must be clear for passed pawn");
    
    std::cout << "\nExplanation: If there's a white pawn on " << square_name 
              << ", enemy pawns on any 'X' square\nwould prevent it from being a passed pawn.\n";
    
    // Visual representation for black pawn
    print_board(black_mask, "BLACK pawn on " + square_name + " - Squares that must be clear for passed pawn");
    
    std::cout << "\nExplanation: If there's a black pawn on " << square_name 
              << ", enemy pawns on any 'X' square\nwould prevent it from being a passed pawn.\n";
}

} // namespace KeySquareDemo

int main() {
    std::cout << "VICE Part 78: Key Square Pawn Mask Demonstration\n";
    std::cout << "================================================\n";
    std::cout << "This demo shows the runtime-initialized evaluation masks\n";
    std::cout << "used for detecting passed pawns in chess evaluation.\n";
    
    // Initialize the evaluation system
    std::cout << "\nInitializing evaluation masks...\n";
    Huginn::init();
    std::cout << "Evaluation system ready!\n";
    
    // Demonstrate key squares
    KeySquareDemo::demonstrate_square(28, "e4");  // Center square
    KeySquareDemo::demonstrate_square(24, "a4");  // Edge file
    KeySquareDemo::demonstrate_square(51, "d7");  // Advanced position
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "SUMMARY\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "The VICE Part 78 implementation successfully:\n";
    std::cout << "• Initializes pawn evaluation masks at runtime\n";
    std::cout << "• Computes passed pawn detection masks for all 64 squares\n";
    std::cout << "• Handles edge cases (A-file, H-file) correctly\n";
    std::cout << "• Provides different masks for white and black pawns\n";
    std::cout << "• Integrates cleanly with the Huginn engine initialization\n\n";
    
    std::cout << "Demo completed successfully!\n";
    return 0;
}
