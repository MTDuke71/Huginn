// VICE Part 78 Demo: Pawn Mask Analysis Tool
// Shows detailed hex values and comparative analysis of pawn masks

#include "../src/evaluation.hpp"
#include "../src/init.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

namespace PawnMaskAnalysis {

// MSVC-compatible popcount function
int popcount(uint64_t x) {
#ifdef _MSC_VER
    return (int)__popcnt64(x);
#else
    return __builtin_popcountll(x);
#endif
}

// Convert square to algebraic notation
std::string sq_to_algebraic(int sq64) {
    if (sq64 < 0 || sq64 > 63) return "invalid";
    int file = sq64 % 8;
    int rank = sq64 / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

// Print mask in hex with bit count
void print_mask_hex(uint64_t mask, const std::string& description) {
    std::cout << std::setw(30) << std::left << description << ": "
              << "0x" << std::hex << std::setw(16) << std::setfill('0') << mask
              << std::dec << " (" << popcount(mask) << " bits)\n";
}

// Analyze a single square's masks
void analyze_square(int sq64) {
    std::string sq_name = sq_to_algebraic(sq64);
    int file = sq64 % 8;
    int rank = sq64 / 8;
    
    std::cout << "\n" << std::string(50, '-') << "\n";
    std::cout << "Square: " << sq_name << " (index: " << sq64 << ")\n";
    std::cout << "File: " << char('a' + file) << " (" << file << "), Rank: " << (rank + 1) << "\n";
    
    uint64_t white_mask = Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[sq64];
    uint64_t black_mask = Huginn::EvalParams::BLACK_PASSED_PAWN_MASKS[sq64];
    
    print_mask_hex(white_mask, "White passed pawn mask");
    print_mask_hex(black_mask, "Black passed pawn mask");
    
    // File and rank masks for reference
    print_mask_hex(Huginn::EvalParams::FILE_MASKS[file], "File mask");
    print_mask_hex(Huginn::EvalParams::RANK_MASKS[rank], "Rank mask");
    
    if (file > 0 && file < 7) {
        print_mask_hex(Huginn::EvalParams::ISOLATED_PAWN_MASKS[file], "Isolated pawn mask");
    }
    
    // Special cases
    if (file == 0) std::cout << "  NOTE: A-file pawn (left edge)\n";
    if (file == 7) std::cout << "  NOTE: H-file pawn (right edge)\n";
    if (rank == 0) std::cout << "  NOTE: First rank (white starting)\n";
    if (rank == 7) std::cout << "  NOTE: Eighth rank (black starting)\n";
}

// Compare masks between different squares
void compare_squares() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "COMPARATIVE ANALYSIS\n";
    
    // Compare center vs edge files
    std::cout << "\n--- Center file (e4) vs Edge file (a4) ---\n";
    int e4 = 28, a4 = 24;  // e4 and a4
    
    std::cout << "e4 white mask: 0x" << std::hex << Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[e4] << std::dec;
    std::cout << " (" << popcount(Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[e4]) << " bits)\n";
    
    std::cout << "a4 white mask: 0x" << std::hex << Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[a4] << std::dec;
    std::cout << " (" << popcount(Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[a4]) << " bits)\n";
    
    // Compare different ranks on same file
    std::cout << "\n--- Same file, different ranks (d2, d4, d6) ---\n";
    int d2 = 11, d4 = 27, d6 = 43;
    
    std::cout << "d2: 0x" << std::hex << Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[d2] << std::dec << "\n";
    std::cout << "d4: 0x" << std::hex << Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[d4] << std::dec << "\n";
    std::cout << "d6: 0x" << std::hex << Huginn::EvalParams::WHITE_PASSED_PAWN_MASKS[d6] << std::dec << "\n";
}

// Show all file and rank masks
void show_file_rank_masks() {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "FILE AND RANK MASKS\n";
    
    std::cout << "\nFile masks:\n";
    for (int file = 0; file < 8; ++file) {
        std::cout << "File " << char('a' + file) << ": 0x" 
                  << std::hex << std::setw(16) << std::setfill('0') 
                  << Huginn::EvalParams::FILE_MASKS[file] << std::dec << "\n";
    }
    
    std::cout << "\nRank masks:\n";
    for (int rank = 0; rank < 8; ++rank) {
        std::cout << "Rank " << (rank + 1) << ": 0x" 
                  << std::hex << std::setw(16) << std::setfill('0') 
                  << Huginn::EvalParams::RANK_MASKS[rank] << std::dec << "\n";
    }
    
    std::cout << "\nIsolated pawn masks:\n";
    for (int file = 0; file < 8; ++file) {
        std::cout << "File " << char('a' + file) << ": 0x" 
                  << std::hex << std::setw(16) << std::setfill('0') 
                  << Huginn::EvalParams::ISOLATED_PAWN_MASKS[file] << std::dec << "\n";
    }
}

// Test specific interesting squares
void demo_interesting_squares() {
    std::cout << "=== VICE Part 78: Detailed Pawn Mask Analysis ===\n";
    std::cout << "This demo shows hex values and bit counts for evaluation masks.\n\n";
    
    // Initialize evaluation system
    std::cout << "Initializing evaluation masks...\n";
    Huginn::init();
    std::cout << "Evaluation masks ready!\n";
    
    // Interesting squares to analyze
    std::vector<int> interesting_squares = {
        0,   // a1 - corner
        7,   // h1 - corner  
        28,  // e4 - center
        24,  // a4 - edge
        31,  // h4 - edge
        11,  // d2 - low rank
        51,  // d7 - high rank
        56,  // a8 - corner
        63   // h8 - corner
    };
    
    for (int sq : interesting_squares) {
        analyze_square(sq);
    }
    
    compare_squares();
    show_file_rank_masks();
}

} // namespace PawnMaskAnalysis

int main() {
    try {
        PawnMaskAnalysis::demo_interesting_squares();
        std::cout << "\nAnalysis completed!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
