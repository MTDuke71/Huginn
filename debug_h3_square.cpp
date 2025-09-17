#include <iostream>
#include <iomanip>
#include "ultra_engine/ultra_position.hpp"
#include "ultra_engine/ultra_move.hpp"

using namespace UltraEngine;

int main() {
    std::cout << "=== H3 SQUARE INVESTIGATION ===\n";
    
    // Kiwipete position
    const std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    UltraPosition pos;
    pos.set_fen(fen);
    
    std::cout << "FEN: " << fen << "\n";
    
    const int h3 = 23; // h3 square
    
    std::cout << "Square h3 (index " << h3 << ") analysis:\n";
    
    // Check what piece is on h3
    const int piece_info = pos.get_piece_at(h3);
    const int color = pos.get_piece_color_at(h3);
    const int piece_type = pos.get_piece_type_at(h3);
    
    std::cout << "Piece info on h3: " << piece_info << "\n";
    std::cout << "Color on h3: " << color << " (" << (color == 0 ? "White" : color == 1 ? "Black" : "Empty") << ")\n";
    std::cout << "Piece type on h3: " << piece_type << "\n";
    
    // Check specific piece boards
    std::cout << "\nPiece board analysis for h3:\n";
    for (int p = 0; p < 6; p++) {
        const char* piece_names[] = {"Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
        
        const uint64_t white_pieces = pos.get_piece_board(0, p);
        const uint64_t black_pieces = pos.get_piece_board(1, p);
        
        const bool white_has = (white_pieces & (1ULL << h3)) != 0;
        const bool black_has = (black_pieces & (1ULL << h3)) != 0;
        
        std::cout << "  " << piece_names[p] << ": White=" << (white_has ? "YES" : "NO") 
                  << ", Black=" << (black_has ? "YES" : "NO") << "\n";
    }
    
    // Check color boards
    const uint64_t white_board = pos.get_color_board(0);
    const uint64_t black_board = pos.get_color_board(1);
    
    const bool in_white_board = (white_board & (1ULL << h3)) != 0;
    const bool in_black_board = (black_board & (1ULL << h3)) != 0;
    
    std::cout << "\nColor board analysis for h3:\n";
    std::cout << "  White color board: " << (in_white_board ? "YES" : "NO") << "\n";
    std::cout << "  Black color board: " << (in_black_board ? "YES" : "NO") << "\n";
    
    // Let's check the FEN interpretation for h3
    std::cout << "\nFEN interpretation:\n";
    std::cout << "Looking at h3 in FEN: " << fen << "\n";
    std::cout << "Rank 3: '2N2Q1p'\n";
    std::cout << "Position h3 should be 'p' (black pawn)\n";
    
    return 0;
}