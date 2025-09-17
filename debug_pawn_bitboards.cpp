#include <iostream>
#include <iomanip>
#include "ultra_engine/ultra_position.hpp"
#include "ultra_engine/ultra_move.hpp"
#include "ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

// Helper to print bitboard
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
    std::cout << "Bitboard value: 0x" << std::hex << bb << std::dec << "\n";
}

int main() {
    std::cout << "=== PAWN CAPTURE BITBOARD ANALYSIS ===\n";
    
    // Kiwipete position
    const std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    UltraPosition pos;
    pos.set_fen(fen);
    
    std::cout << "FEN: " << fen << "\n";
    
    // Get white pawns
    const uint64_t white_pawns = pos.get_piece_board(0, UltraPosition::Pawn);
    const uint64_t black_pieces = pos.get_color_board(1);
    
    print_bitboard(white_pawns, "White Pawns");
    print_bitboard(black_pieces, "Black Pieces");
    
    // Analyze white pawn captures
    std::cout << "\n=== WHITE PAWN CAPTURE ANALYSIS ===\n";
    
    // Left captures (pawns << 7) - northwest direction for white
    const uint64_t left_shift = white_pawns << 7;
    const uint64_t left_mask = 0xFEFEFEFEFEFEFEFEULL; // Not H-file
    const uint64_t left_captures = left_shift & black_pieces & left_mask;
    
    print_bitboard(left_shift, "Left Shift (pawns << 7)");
    print_bitboard(left_mask, "Left Mask (Not H-file)");
    print_bitboard(left_captures, "Left Captures");
    
    // Right captures (pawns << 9) - northeast direction for white  
    const uint64_t right_shift = white_pawns << 9;
    const uint64_t right_mask = 0x7F7F7F7F7F7F7F7FULL; // Not A-file
    const uint64_t right_captures = right_shift & black_pieces & right_mask;
    
    print_bitboard(right_shift, "Right Shift (pawns << 9)");
    print_bitboard(right_mask, "Right Mask (Not A-file)");
    print_bitboard(right_captures, "Right Captures");
    
    // Specific analysis for g2h3
    std::cout << "\n=== G2H3 SPECIFIC ANALYSIS ===\n";
    const int g2 = 14; // g2 square
    const int h3 = 23; // h3 square
    
    std::cout << "g2 square: " << g2 << ", bit position: " << (1ULL << g2) << "\n";
    std::cout << "h3 square: " << h3 << ", bit position: " << (1ULL << h3) << "\n";
    
    const bool g2_has_white_pawn = (white_pawns & (1ULL << g2)) != 0;
    const bool h3_has_black_piece = (black_pieces & (1ULL << h3)) != 0;
    
    std::cout << "g2 has white pawn: " << (g2_has_white_pawn ? "YES" : "NO") << "\n";
    std::cout << "h3 has black piece: " << (h3_has_black_piece ? "YES" : "NO") << "\n";
    
    // For g2h3, it's a right capture (northeast): g2(14) -> h3(23) = shift by 9
    const uint64_t g2_bit = 1ULL << g2;
    const uint64_t g2_right_shift = g2_bit << 9;
    const uint64_t h3_bit = 1ULL << h3;
    
    std::cout << "g2 bit: 0x" << std::hex << g2_bit << std::dec << "\n";
    std::cout << "g2 << 9: 0x" << std::hex << g2_right_shift << std::dec << "\n";
    std::cout << "h3 bit: 0x" << std::hex << h3_bit << std::dec << "\n";
    std::cout << "g2 << 9 == h3: " << ((g2_right_shift == h3_bit) ? "YES" : "NO") << "\n";
    
    // Check if g2 is affected by the A-file mask
    const bool g2_blocked_by_mask = (g2_bit & right_mask) == 0;
    std::cout << "g2 blocked by A-file mask: " << (g2_blocked_by_mask ? "YES - BUG!" : "NO") << "\n";
    
    // Check the full capture calculation for g2h3
    const uint64_t g2_capture_test = (g2_bit << 9) & black_pieces & right_mask;
    std::cout << "g2h3 capture test result: 0x" << std::hex << g2_capture_test << std::dec << "\n";
    std::cout << "g2h3 should be captured: " << ((g2_capture_test != 0) ? "YES" : "NO") << "\n";
    
    return 0;
}