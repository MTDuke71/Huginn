// Simple VICE Part 78 Demo: Show pawn masks for key squares
// Minimal demo showing the essential mask functionality

#include "../src/evaluation.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <iomanip>
#include <bitset>

namespace SimplePawnDemo {

// MSVC-compatible popcount function
int popcount(uint64_t x) {
#ifdef _MSC_VER
    return (int)__popcnt64(x);
#else
    return __builtin_popcountll(x);
#endif
}

// Print a single square's masks in a compact format
void print_square_masks(int square, const std::string& name) {
    std::cout << "\n=== " << name << " (square " << square << ") ===\n";
    
    uint64_t white_mask = Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[square];
    uint64_t black_mask = Huginn::EvalParams::BLACK_PASSED_PAWN_MASKS[square];
    
    std::cout << "White pawn mask: 0x" << std::hex << white_mask << std::dec 
              << " (" << popcount(white_mask) << " bits)\n";
    std::cout << "Black pawn mask: 0x" << std::hex << black_mask << std::dec 
              << " (" << popcount(black_mask) << " bits)\n";
}

// Simple visual board for a mask
void print_simple_board(uint64_t mask, const std::string& title) {
    std::cout << "\n" << title << ":\n";
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << (rank + 1) << " ";
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            std::cout << ((mask & (1ULL << sq)) ? "X" : ".") << " ";
        }
        std::cout << "\n";
    }
    std::cout << "  a b c d e f g h\n";
}

} // namespace SimplePawnDemo

int main() {
    std::cout << "VICE Part 78: Simple Pawn Mask Demo\n";
    std::cout << "===================================\n";
    
    // Initialize the evaluation system
    std::cout << "Initializing evaluation masks...\n";
    Huginn::init();
    std::cout << "Done!\n";
    
    // Show masks for a few key squares
    SimplePawnDemo::print_square_masks(28, "e4 (center square)");
    SimplePawnDemo::print_square_masks(24, "a4 (edge file)");
    SimplePawnDemo::print_square_masks(11, "d2 (starting area)");
    SimplePawnDemo::print_square_masks(51, "d7 (advanced)");
    
    // Show visual representation for e4
    std::cout << "\nVisual representation for e4:\n";
    SimplePawnDemo::print_simple_board(
        Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[28], 
        "White pawn on e4 - X marks squares that must be clear"
    );
    
    SimplePawnDemo::print_simple_board(
        Huginn::EvalParams::BLACK_PASSED_PAWN_MASKS[28], 
        "Black pawn on e4 - X marks squares that must be clear"
    );
    
    // Show some basic file/rank masks
    std::cout << "\nFile and rank mask examples:\n";
    SimplePawnDemo::print_simple_board(
        Huginn::EvalParams::FILE_MASKS[4], 
        "E-file mask"
    );
    
    SimplePawnDemo::print_simple_board(
        Huginn::EvalParams::RANK_MASKS[3], 
        "4th rank mask"
    );
    
    std::cout << "\nDemo complete!\n";
    return 0;
}
