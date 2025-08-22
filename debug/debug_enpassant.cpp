#include <iostream>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "init.hpp"

int main() {
    Huginn::init();
    
    std::cout << "=== En Passant Debug Tool ===" << std::endl;
    
    // Set up position after a2a4
    Position pos;
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/2N2Q1p/1PPBBPPP/R3K2R b KQkq a3 0 1";
    
    if (!pos.set_from_fen(fen)) {
        std::cout << "Failed to parse FEN" << std::endl;
        return 1;
    }
    
    std::cout << "FEN: " << fen << std::endl;
    std::cout << "En passant square: " << pos.ep_square << std::endl;
    
    if (pos.ep_square != -1) {
        // Convert ep_square to algebraic notation
        int file = (pos.ep_square % 10) - 1;
        int rank = (pos.ep_square / 10) - 2;
        char file_char = 'a' + file;
        char rank_char = '1' + rank;
        std::cout << "En passant square (algebraic): " << file_char << rank_char << std::endl;
    } else {
        std::cout << "No en passant square set" << std::endl;
    }
    
    // Check what piece is on b4
    int b4_square = 32; // b4 in mailbox120 (rank 4 = row 6, b = col 2, so 6*10 + 2 = 62? Let me calculate...)
    // Actually: rank 4 is internal rank 3 (0-based), file b is 1, so (3+2)*10 + (1+1) = 5*10 + 2 = 52? 
    // Let me try different values
    
    // Find all pieces on rank 4 (which should be rank index 3, so row 5 in mailbox120)
    std::cout << "\nPieces on rank 4:" << std::endl;
    for (int file = 0; file < 8; file++) {
        int square = 50 + file + 1; // rank 4 = row 5 in mailbox120, files a-h = 1-8
        char file_char = 'a' + file;
        std::cout << file_char << "4 (square " << square << "): " << int(pos.at(square)) << std::endl;
    }
    
    // Now check b4 specifically
    b4_square = 52; // b4 should be at square 52
    std::cout << "\nPiece on b4 (square " << b4_square << "): " << int(pos.at(b4_square)) << std::endl;
    
    // Generate moves for the pawn on b4 specifically
    MoveList all_moves;
    generate_legal_moves(pos, all_moves);
    
    std::cout << "\nMoves from b4:" << std::endl;
    int move_count = 0;
    for (const auto& move : all_moves.v) {
        if (move.get_from() == b4_square) {
            move_count++;
            int to = move.get_to();
            int to_file = (to % 10) - 1;
            int to_rank = (to / 10) - 2;
            char to_file_char = 'a' + to_file;
            char to_rank_char = '1' + to_rank;
            
            std::cout << move_count << ". b4" << to_file_char << to_rank_char;
            if (move.is_en_passant()) std::cout << " (en passant)";
            if (move.is_capture()) std::cout << " (capture)";
            std::cout << std::endl;
        }
    }
    
    if (move_count == 0) {
        std::cout << "No moves found from b4!" << std::endl;
    }
    
    return 0;
}
