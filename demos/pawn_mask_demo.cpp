// VICE Part 78 Demo: Pawn Mask Visualization
// This demo shows the runtime-initialized evaluation masks for passed pawns

#include "../src/evaluation.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <iomanip>
#include <string>

namespace PawnMaskDemo {

// Convert square index to algebraic notation (e.g., 0 -> "a1", 63 -> "h8")
std::string square_to_algebraic(int sq64) {
    if (sq64 < 0 || sq64 > 63) return "invalid";
    
    int file = sq64 % 8;
    int rank = sq64 / 8;
    
    char file_char = 'a' + file;
    char rank_char = '1' + rank;
    
    return std::string(1, file_char) + std::string(1, rank_char);
}

// Convert algebraic notation to square index (e.g., "e4" -> 28)
int algebraic_to_square(const std::string& algebraic) {
    if (algebraic.length() != 2) return -1;
    
    char file_char = algebraic[0];
    char rank_char = algebraic[1];
    
    if (file_char < 'a' || file_char > 'h') return -1;
    if (rank_char < '1' || rank_char > '8') return -1;
    
    int file = file_char - 'a';
    int rank = rank_char - '1';
    
    return rank * 8 + file;
}

// Print a bitboard as a visual 8x8 chess board
void print_bitboard(uint64_t bitboard, const std::string& title) {
    std::cout << "\n" << title << ":\n";
    std::cout << "  +---+---+---+---+---+---+---+---+\n";
    
    // Print from rank 8 down to rank 1 (top to bottom)
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << (rank + 1) << " |";
        
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            bool bit_set = (bitboard & (1ULL << square)) != 0;
            std::cout << " " << (bit_set ? "X" : " ") << " |";
        }
        
        std::cout << "\n  +---+---+---+---+---+---+---+---+\n";
    }
    
    std::cout << "    a   b   c   d   e   f   g   h\n";
}

// Demo function to show masks for specific squares
void demo_pawn_masks() {
    std::cout << "=== VICE Part 78: Pawn Mask Demonstration ===\n";
    std::cout << "This demo shows the evaluation masks used for passed pawn detection.\n";
    std::cout << "X marks squares that must be clear for a pawn to be considered 'passed'.\n\n";
    
    // Initialize the engine to set up evaluation masks
    std::cout << "Initializing evaluation masks...\n";
    Huginn::init();
    std::cout << "Evaluation masks initialized successfully!\n";
    
    // Demo squares to show
    std::vector<std::string> demo_squares = {
        "e4",  // Central pawn
        "a4",  // A-file pawn (edge case)
        "h5",  // H-file pawn (edge case)  
        "d2",  // Starting rank pawn
        "f6",  // Advanced pawn
        "b7"   // Near-promotion pawn
    };
    
    for (const std::string& square_name : demo_squares) {
        int sq64 = algebraic_to_square(square_name);
        if (sq64 < 0) continue;
        
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "Square: " << square_name << " (index: " << sq64 << ")\n";
        
        // Show white passed pawn mask
        uint64_t white_mask = Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[sq64];
        std::string white_title = "White pawn on " + square_name + " - squares that must be clear";
        print_bitboard(white_mask, white_title);
        
        // Show black passed pawn mask  
        uint64_t black_mask = Huginn::EvalParams::BLACK_PASSED_PAWN_MASKS[sq64];
        std::string black_title = "Black pawn on " + square_name + " - squares that must be clear";
        print_bitboard(black_mask, black_title);
        
        // Analysis
        std::cout << "\nAnalysis for " << square_name << ":\n";
        // Manual popcount for cross-platform compatibility
        int white_count = 0;
        for (uint64_t temp = white_mask; temp; temp >>= 1) if (temp & 1) white_count++;
        int black_count = 0;
        for (uint64_t temp = black_mask; temp; temp >>= 1) if (temp & 1) black_count++;
        std::cout << "- White mask covers " << white_count << " squares\n";
        std::cout << "- Black mask covers " << black_count << " squares\n";
        
        int file = sq64 % 8;
        int rank = sq64 / 8;
        std::cout << "- File: " << char('a' + file) << " (" << file << "), Rank: " << (rank + 1) << " (" << rank << ")\n";
        
        if (file == 0) std::cout << "- Edge file (A): Only right side checked\n";
        else if (file == 7) std::cout << "- Edge file (H): Only left side checked\n";
        else std::cout << "- Interior file: Both sides checked\n";
    }
    
    // Show file and rank masks
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "BONUS: File and Rank Masks\n";
    
    // Show D-file mask
    print_bitboard(Huginn::EvalParams::FILE_MASKS[3], "D-file mask (file index 3)");
    
    // Show 4th rank mask  
    print_bitboard(Huginn::EvalParams::RANK_MASKS[3], "4th rank mask (rank index 3)");
    
    // Show isolated pawn mask for E-file
    print_bitboard(Huginn::EvalParams::ISOLATED_PAWN_MASKS[4], "Isolated pawn mask for E-file (adjacent files D and F)");
}

// Interactive mode - let user input squares
void interactive_mode() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Interactive Mode - Enter squares to see their masks\n";
    std::cout << "Enter squares like 'e4', 'a1', 'h8', etc. Type 'quit' to exit.\n\n";
    
    std::string input;
    while (true) {
        std::cout << "Enter square (or 'quit'): ";
        std::cin >> input;
        
        if (input == "quit" || input == "q" || input == "exit") {
            break;
        }
        
        int sq64 = algebraic_to_square(input);
        if (sq64 < 0) {
            std::cout << "Invalid square: " << input << ". Use format like 'e4'.\n";
            continue;
        }
        
        std::cout << "\n--- Masks for " << input << " ---\n";
        
        uint64_t white_mask = Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[sq64];
        uint64_t black_mask = Huginn::EvalParams::BLACK_PASSED_PAWN_MASKS[sq64];
        
        print_bitboard(white_mask, "White pawn passed mask");
        print_bitboard(black_mask, "Black pawn passed mask");
        
        // Manual popcount for cross-platform compatibility  
        int white_count = 0;
        for (uint64_t temp = white_mask; temp; temp >>= 1) if (temp & 1) white_count++;
        int black_count = 0;
        for (uint64_t temp = black_mask; temp; temp >>= 1) if (temp & 1) black_count++;
        std::cout << "White mask bits: " << white_count << "\n";
        std::cout << "Black mask bits: " << black_count << "\n";
    }
}

} // namespace PawnMaskDemo

int main() {
    try {
        PawnMaskDemo::demo_pawn_masks();
        PawnMaskDemo::interactive_mode();
        
        std::cout << "\nDemo completed successfully!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
