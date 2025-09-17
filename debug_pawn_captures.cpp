/**
 * @file debug_pawn_captures.cpp
 * @brief Debug pawn capture generation specifically for g2h3 move
 */
#include <iostream>
#include <bitset>
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_move.hpp"
#include "../ultra_engine/ultra_attacks.hpp"
#include "../ultra_engine/ultra_pawn_gen.hpp"

using namespace UltraEngine;

// Convert square to algebraic notation
std::string square_to_algebraic(int square) {
    int file = square % 8;  // 0-7 for a-h
    int rank = square / 8;  // 0-7 for 1-8
    char file_char = 'a' + file;
    char rank_char = '1' + rank;
    return std::string(1, file_char) + std::string(1, rank_char);
}

// Convert move to algebraic notation
std::string move_to_algebraic(const UltraMove& move) {
    return square_to_algebraic(move.from()) + square_to_algebraic(move.to());
}

int main() {
    UltraAttacks::initialize();
    
    // Correct Kiwipete position
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    UltraPosition pos;
    if (!pos.set_fen(fen)) {
        std::cout << "Failed to parse FEN" << std::endl;
        return 1;
    }
    
    std::cout << "=== PAWN CAPTURE DEBUG: g2h3 ===" << std::endl;
    std::cout << "FEN: " << fen << std::endl;
    std::cout << std::endl;
    
    // Check what pieces are on the relevant squares
    int g2_square = 14; // g2 = rank 1, file 6 = 1*8 + 6 = 14
    int h3_square = 23; // h3 = rank 2, file 7 = 2*8 + 7 = 23
    
    std::cout << "Square verification:" << std::endl;
    std::cout << "  g2 (square " << g2_square << "): piece=" << pos.get_piece_at(g2_square) << std::endl;
    std::cout << "  h3 (square " << h3_square << "): piece=" << pos.get_piece_at(h3_square) << std::endl;
    
    // Decode piece info
    int g2_piece = pos.get_piece_at(g2_square);
    int h3_piece = pos.get_piece_at(h3_square);
    
    if (g2_piece != -1) {
        int g2_color = (g2_piece >> 3) & 1;
        int g2_type = g2_piece & 7;
        std::cout << "  g2: " << (g2_color == 0 ? "White" : "Black") << " piece type " << g2_type;
        if (g2_type == 0) std::cout << " (PAWN)";
        std::cout << std::endl;
    } else {
        std::cout << "  g2: EMPTY (ERROR!)" << std::endl;
    }
    
    if (h3_piece != -1) {
        int h3_color = (h3_piece >> 3) & 1;
        int h3_type = h3_piece & 7;
        std::cout << "  h3: " << (h3_color == 0 ? "White" : "Black") << " piece type " << h3_type;
        if (h3_type == 0) std::cout << " (PAWN)";
        std::cout << std::endl;
    } else {
        std::cout << "  h3: EMPTY (ERROR!)" << std::endl;
    }
    
    std::cout << std::endl;
    
    // Generate all pawn moves for white
    std::cout << "=== WHITE PAWN MOVES GENERATION ===" << std::endl;
    UltraMoveList pawn_moves;
    
    // Get white pawn moves using UltraPawnGen (if available)
    int white_color = 0;
    
    // Generate all moves and filter for pawn moves
    UltraMoveList all_moves;
    pos.generate_all_moves(all_moves);
    
    std::cout << "All white pawn moves found:" << std::endl;
    int pawn_count = 0;
    
    for (int i = 0; i < all_moves.size(); i++) {
        const UltraMove& move = all_moves[i];
        
        if (pos.is_legal_move(move)) {
            // Check if this is a pawn move by examining the source square
            int from_piece = pos.get_piece_at(move.from());
            if (from_piece != -1) {
                int piece_color = (from_piece >> 3) & 1;
                int piece_type = from_piece & 7;
                
                if (piece_color == 0 && piece_type == 0) { // White pawn
                    pawn_count++;
                    std::string move_str = move_to_algebraic(move);
                    std::cout << "  " << pawn_count << ": " << move_str;
                    
                    if (move_str == "g2h3") {
                        std::cout << " *** THIS IS THE MISSING MOVE! ***";
                    }
                    
                    // Check if it's a capture
                    int to_piece = pos.get_piece_at(move.to());
                    if (to_piece != -1) {
                        std::cout << " (CAPTURE)";
                    }
                    
                    std::cout << std::endl;
                }
            }
        }
    }
    
    std::cout << std::endl;
    std::cout << "Total white pawn moves found: " << pawn_count << std::endl;
    
    // Check if g2h3 specifically exists
    bool found_g2h3 = false;
    for (int i = 0; i < all_moves.size(); i++) {
        const UltraMove& move = all_moves[i];
        if (pos.is_legal_move(move)) {
            if (move.from() == g2_square && move.to() == h3_square) {
                found_g2h3 = true;
                std::cout << "✓ Found g2h3 move in generated moves!" << std::endl;
                break;
            }
        }
    }
    
    if (!found_g2h3) {
        std::cout << "✗ g2h3 move NOT found in generated moves!" << std::endl;
        std::cout << "This confirms the bug is in pawn capture generation." << std::endl;
    }
    
    return 0;
}