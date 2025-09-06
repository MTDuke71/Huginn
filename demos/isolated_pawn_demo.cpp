// VICE Part 78 Demo: Isolated Pawn Detection
// Shows how the isolated pawn masks work for pawn structure evaluation

#include "../src/evaluation.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <iomanip>

namespace IsolatedPawnDemo {

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

void demonstrate_isolated_pawn(int file_index, char file_letter) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "ISOLATED PAWN DETECTION: " << file_letter << "-file\n";
    std::cout << std::string(60, '=') << "\n";
    
    uint64_t isolated_mask = Huginn::EvalParams::ISOLATED_PAWN_MASKS[file_index];
    
    std::cout << "\nHex value: 0x" << std::hex << isolated_mask << std::dec 
              << " (" << popcount(isolated_mask) << " bits set)\n";
    
    print_board(isolated_mask, file_letter + std::string("-file isolated pawn mask - Adjacent files that must be checked"));
    
    std::cout << "\nExplanation: A pawn on the " << file_letter 
              << "-file is ISOLATED if there are NO friendly pawns\n";
    std::cout << "on any of the 'X' squares (the adjacent files).\n";
    
    // Show which files are adjacent
    if (file_index == 0) {
        std::cout << "NOTE: A-file only checks B-file (right edge case)\n";
    } else if (file_index == 7) {
        std::cout << "NOTE: H-file only checks G-file (left edge case)\n";
    } else {
        char left_file = 'a' + file_index - 1;
        char right_file = 'a' + file_index + 1;
        std::cout << "NOTE: " << file_letter << "-file checks " << left_file 
                  << "-file and " << right_file << "-file\n";
    }
}

void demonstrate_isolated_detection_examples() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "PRACTICAL EXAMPLES\n";
    std::cout << std::string(60, '=') << "\n";
    
    std::cout << "\nExample 1: Is a D-file pawn isolated?\n";
    std::cout << "-------------------------------------\n";
    std::cout << "Step 1: Get the D-file isolated pawn mask\n";
    uint64_t d_file_mask = Huginn::EvalParams::ISOLATED_PAWN_MASKS[3]; // D-file = index 3
    std::cout << "D-file mask: 0x" << std::hex << d_file_mask << std::dec << "\n";
    
    std::cout << "\nStep 2: Check if any friendly pawns exist on C-file OR E-file\n";
    std::cout << "Step 3: If (friendly_pawns & d_file_mask) == 0, then the D-pawn is ISOLATED\n";
    
    print_board(d_file_mask, "D-file isolation check - must have friendly pawns here to NOT be isolated");
    
    std::cout << "\nExample 2: Edge file isolation\n";
    std::cout << "------------------------------\n";
    std::cout << "A-file pawns can only be supported by B-file pawns:\n";
    uint64_t a_file_mask = Huginn::EvalParams::ISOLATED_PAWN_MASKS[0];
    print_board(a_file_mask, "A-file isolation check - only B-file can provide support");
    
    std::cout << "\nH-file pawns can only be supported by G-file pawns:\n";
    uint64_t h_file_mask = Huginn::EvalParams::ISOLATED_PAWN_MASKS[7];
    print_board(h_file_mask, "H-file isolation check - only G-file can provide support");
}

void show_all_isolated_masks() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "ALL ISOLATED PAWN MASKS\n";
    std::cout << std::string(60, '=') << "\n";
    
    for (int file = 0; file < 8; ++file) {
        char file_letter = 'a' + file;
        uint64_t mask = Huginn::EvalParams::ISOLATED_PAWN_MASKS[file];
        
        std::cout << file_letter << "-file: 0x" << std::hex << std::setw(16) << std::setfill('0') 
                  << mask << std::dec << " (" << std::setw(2) << popcount(mask) << " bits)";
        
        if (file == 0) std::cout << " [only checks b-file]";
        else if (file == 7) std::cout << " [only checks g-file]";
        else {
            char left = 'a' + file - 1;
            char right = 'a' + file + 1;
            std::cout << " [checks " << left << "-file + " << right << "-file]";
        }
        std::cout << "\n";
    }
}

} // namespace IsolatedPawnDemo

int main() {
    std::cout << "VICE Part 78: Isolated Pawn Mask Demonstration\n";
    std::cout << "===============================================\n";
    std::cout << "This demo shows how isolated pawns are detected using\n";
    std::cout << "bitboard masks for adjacent files.\n";
    
    // Initialize the evaluation system
    std::cout << "\nInitializing evaluation masks...\n";
    Huginn::init();
    std::cout << "Evaluation system ready!\n";
    
    // Demonstrate key files
    IsolatedPawnDemo::demonstrate_isolated_pawn(0, 'a');  // Edge case
    IsolatedPawnDemo::demonstrate_isolated_pawn(3, 'd');  // Center file
    IsolatedPawnDemo::demonstrate_isolated_pawn(4, 'e');  // Center file
    IsolatedPawnDemo::demonstrate_isolated_pawn(7, 'h');  // Edge case
    
    // Show practical examples
    IsolatedPawnDemo::demonstrate_isolated_detection_examples();
    
    // Show all masks
    IsolatedPawnDemo::show_all_isolated_masks();
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "ISOLATED PAWN ALGORITHM SUMMARY\n";
    std::cout << std::string(60, '=') << "\n";
    std::cout << "To check if a pawn on file F is isolated:\n";
    std::cout << "1. Get friendly_pawns bitboard for the same color\n";
    std::cout << "2. Get ISOLATED_PAWN_MASKS[F] (adjacent files mask)\n";
    std::cout << "3. If (friendly_pawns & ISOLATED_PAWN_MASKS[F]) == 0:\n";
    std::cout << "   -> The pawn is ISOLATED (no friendly pawns on adjacent files)\n";
    std::cout << "4. Apply evaluation penalty for isolated pawns\n\n";
    
    std::cout << "The VICE Part 78 implementation provides these masks\n";
    std::cout << "for efficient bitboard-based isolated pawn detection!\n\n";
    
    std::cout << "Demo completed successfully!\n";
    return 0;
}
