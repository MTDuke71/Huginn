/**
 * Pawn Table Comparison
 * 
 * Compares the provided pawn table with our current implementation
 * to analyze the differences and their potential impact.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/evaluation.hpp"
#include <iostream>
#include <iomanip>

// Provided pawn table from user
const int ProvidedPawnTable[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    10, 10, 0, -10, -10, 0, 10, 10,
    5,  0,  0,  5,  5,  0,  0,  5,
    0,  0, 10, 20, 20, 10,  0,  0,
    5,  5,  5, 10, 10,  5,  5,  5,
    10, 10, 10, 20, 20, 10, 10, 10,
    20, 20, 20, 30, 30, 20, 20, 20,
    0,  0,  0,  0,  0,  0,  0,  0
};

void compare_pawn_tables() {
    using namespace Huginn;
    
    std::cout << "Pawn Table Comparison Analysis\n";
    std::cout << "==============================\n\n";
    
    std::cout << "Square-by-Square Comparison:\n";
    std::cout << "============================\n";
    std::cout << "Rank File | Provided | Current | Difference | Analysis\n";
    std::cout << "----------|----------|---------|------------|----------\n";
    
    int total_difference = 0;
    int max_difference = 0;
    int significant_differences = 0;
    
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int sq64 = rank * 8 + file;
            int provided_value = ProvidedPawnTable[sq64];
            int current_value = EvalParams::PAWN_TABLE[sq64];
            int difference = current_value - provided_value;
            
            char file_char = 'a' + file;
            char rank_char = '1' + rank;
            
            std::cout << "  " << rank_char << "    " << file_char << "   |";
            std::cout << std::setw(8) << provided_value << " |";
            std::cout << std::setw(7) << current_value << " |";
            std::cout << std::setw(10) << difference << " |";
            
            // Analysis
            if (difference == 0) {
                std::cout << " Same";
            } else if (abs(difference) <= 5) {
                std::cout << " Minor diff";
            } else if (abs(difference) <= 15) {
                std::cout << " Moderate diff";
                significant_differences++;
            } else {
                std::cout << " MAJOR diff";
                significant_differences++;
            }
            
            std::cout << "\n";
            
            total_difference += abs(difference);
            max_difference = std::max(max_difference, abs(difference));
        }
    }
    
    std::cout << "\nStatistical Analysis:\n";
    std::cout << "=====================\n";
    std::cout << "Total absolute difference: " << total_difference << "\n";
    std::cout << "Average difference per square: " << (total_difference / 64.0) << "\n";
    std::cout << "Maximum single difference: " << max_difference << "\n";
    std::cout << "Significant differences (>5): " << significant_differences << "\n\n";
    
    std::cout << "Strategic Pattern Analysis:\n";
    std::cout << "===========================\n";
    
    std::cout << "Rank 1 (1st rank): Both have 0 values - pawns can't be here\n";
    std::cout << "Rank 2 (2nd rank):\n";
    std::cout << "  Provided: [10, 10, 0, -10, -10, 0, 10, 10] - Penalizes center pawns\n";
    std::cout << "  Current:  [5, 10, 10, -20, -20, 10, 10, 5] - Stronger center penalty\n";
    
    std::cout << "Rank 3 (3rd rank):\n";
    std::cout << "  Provided: [5, 0, 0, 5, 5, 0, 0, 5] - Corner bonuses\n";
    std::cout << "  Current:  [5, -5, -10, 0, 0, -10, -5, 5] - Penalizes b/g files\n";
    
    std::cout << "Rank 4 (4th rank):\n";
    std::cout << "  Provided: [0, 0, 10, 20, 20, 10, 0, 0] - Center advancement bonus\n";
    std::cout << "  Current:  [0, 0, 0, 25, 25, 0, 0, 0] - Focuses only on d/e files\n";
    
    std::cout << "Rank 5 (5th rank):\n";
    std::cout << "  Provided: [5, 5, 5, 10, 10, 5, 5, 5] - Uniform advancement\n";
    std::cout << "  Current:  [5, 5, 15, 30, 30, 15, 5, 5] - Strong center preference\n";
    
    std::cout << "Rank 6 (6th rank):\n";
    std::cout << "  Provided: [10, 10, 10, 20, 20, 10, 10, 10] - Moderate advancement\n";
    std::cout << "  Current:  [10, 10, 25, 35, 35, 25, 10, 10] - Heavy center weighting\n";
    
    std::cout << "Rank 7 (7th rank):\n";
    std::cout << "  Provided: [20, 20, 20, 30, 30, 20, 20, 20] - Good promotion values\n";
    std::cout << "  Current:  [50, 50, 50, 50, 50, 50, 50, 50] - MASSIVE promotion bonus\n";
    
    std::cout << "Rank 8 (8th rank): Both have 0 values - pawns promote here\n\n";
    
    std::cout << "Key Differences:\n";
    std::cout << "================\n";
    std::cout << "1. PROMOTION RANK: Our table gives +50 vs provided +20-30\n";
    std::cout << "   - Our approach: Massive promotion incentive\n";
    std::cout << "   - Provided: More conservative promotion values\n\n";
    
    std::cout << "2. CENTER CONTROL: Our table is more center-focused\n";
    std::cout << "   - Our 5th/6th rank d/e pawns: +30/+35\n";
    std::cout << "   - Provided 5th/6th rank d/e: +10/+20\n\n";
    
    std::cout << "3. WING DEVELOPMENT: Different philosophies\n";
    std::cout << "   - Provided: Encourages c/f file advancement (rank 4: +10)\n";
    std::cout << "   - Our: Focuses purely on d/e files (rank 4: c/f=0)\n\n";
    
    std::cout << "4. EARLY DEVELOPMENT PENALTIES:\n";
    std::cout << "   - Provided: -10 penalty for early d/e pawn moves\n";
    std::cout << "   - Our: -20 penalty (stronger discouragement)\n\n";
    
    std::cout << "IMPACT ON VICE TEST:\n";
    std::cout << "====================\n";
    std::cout << "Testing how this would affect the 1.e4 c6 position...\n";
}

void test_vice_with_provided_table() {
    using namespace Huginn;
    
    std::cout << "\nVICE Test with Provided Pawn Table:\n";
    std::cout << "===================================\n";
    
    // Position after 1.e4 c6
    std::string e4_c6_fen = "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";
    Position original_pos;
    original_pos.set_from_fen(e4_c6_fen);
    
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    
    // Calculate PST scores with provided table
    int orig_pst_provided = 0;
    int mirr_pst_provided = 0;
    
    for (int sq = 21; sq <= 98; ++sq) {
        if (original_pos.board[sq] == Piece::Offboard || original_pos.board[sq] == Piece::None) continue;
        
        Piece piece = original_pos.board[sq];
        if (type_of(piece) == PieceType::Pawn) {
            Color piece_color = color_of(piece);
            int sq64 = MAILBOX_MAPS.to64[sq];
            
            if (sq64 >= 0) {
                int pst_sq64 = sq64;
                if (piece_color == Color::Black) {
                    pst_sq64 = ((7 - (sq64 / 8)) * 8) + (sq64 % 8);
                }
                
                int provided_value = ProvidedPawnTable[pst_sq64];
                
                if (piece_color == Color::White) {
                    orig_pst_provided += provided_value;
                } else {
                    orig_pst_provided -= provided_value;
                }
            }
        }
    }
    
    for (int sq = 21; sq <= 98; ++sq) {
        if (mirrored_pos.board[sq] == Piece::Offboard || mirrored_pos.board[sq] == Piece::None) continue;
        
        Piece piece = mirrored_pos.board[sq];
        if (type_of(piece) == PieceType::Pawn) {
            Color piece_color = color_of(piece);
            int sq64 = MAILBOX_MAPS.to64[sq];
            
            if (sq64 >= 0) {
                int pst_sq64 = sq64;
                if (piece_color == Color::Black) {
                    pst_sq64 = ((7 - (sq64 / 8)) * 8) + (sq64 % 8);
                }
                
                int provided_value = ProvidedPawnTable[pst_sq64];
                
                if (piece_color == Color::White) {
                    mirr_pst_provided += provided_value;
                } else {
                    mirr_pst_provided -= provided_value;
                }
            }
        }
    }
    
    std::cout << "Pawn PST with Provided Table:\n";
    std::cout << "Original position: " << orig_pst_provided << "\n";
    std::cout << "Mirrored position: " << mirr_pst_provided << "\n";
    std::cout << "Difference: " << (orig_pst_provided - mirr_pst_provided) << "\n";
    std::cout << "Would this improve VICE symmetry? " << (abs(orig_pst_provided - mirr_pst_provided) < 20 ? "YES" : "NO") << "\n";
}

int main() {
    Huginn::init();
    compare_pawn_tables();
    test_vice_with_provided_table();
    return 0;
}
