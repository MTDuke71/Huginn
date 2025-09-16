#include <iostream>
#include "src/bitboard_position.h"
#include "src/bitboard_movegen_pure.h"
#include "src/bitboard_attacks.h"
#include "src/init.h"

int main() {
    std::cout << "=== Analyze After e2b5 Move ===" << std::endl;
    
    init_all();
    
    // Set up Kiwipete position
    BitboardPosition pos;
    pos.parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    
    std::cout << "\nInitial position: r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1" << std::endl;
    
    // Make the e2b5 move  
    BitboardMoveList moves;
    BitboardMoveGen movegen;
    movegen.generate_moves(pos, moves);
    
    bool move_found = false;
    for (int i = 0; i < moves.count; i++) {
        int from_64 = moves.moves[i].from_square;
        int to_64 = moves.moves[i].to_square;
        
        if (from_64 == 12 && to_64 == 33) { // e2 to b5
            pos.make_move(moves.moves[i]);
            move_found = true;
            break;
        }
    }
    
    if (!move_found) {
        std::cout << "Error: e2b5 move not found!" << std::endl;
        return 1;
    }
    
    std::cout << "\nAfter e2b5 move:" << std::endl;
    std::cout << "New position FEN: " << pos.to_fen() << std::endl;
    
    // Generate moves for Black
    moves.count = 0;
    movegen.generate_moves(pos, moves);
    
    std::cout << "\nBlack's moves (" << moves.count << " total):" << std::endl;
    for (int i = 0; i < moves.count; i++) {
        int from_64 = moves.moves[i].from_square;
        int to_64 = moves.moves[i].to_square;
        
        std::string from_str = "";
        std::string to_str = "";
        
        // Convert square numbers to algebraic notation
        char files[] = "abcdefgh";
        char ranks[] = "12345678";
        from_str = std::string(1, files[from_64 % 8]) + ranks[from_64 / 8];
        to_str = std::string(1, files[to_64 % 8]) + ranks[to_64 / 8];
        
        std::cout << "  " << from_str << to_str;
        if (moves.moves[i].is_castling) {
            std::cout << " (castling)";
        }
        if (pos.is_square_occupied(to_64)) {
            std::cout << " (capture)";
        }
        std::cout << std::endl;
    }
    
    // Check specific moves that might be problematic
    std::cout << "\n=== Attack Analysis ===" << std::endl;
    
    // Check king squares for attacks
    int black_king_pos = pos.king_square_64[1];
    std::cout << "Black king on square " << black_king_pos << " (" << 
        files[black_king_pos % 8] << ranks[black_king_pos / 8] << ")" << std::endl;
    
    // Check if any castling moves are being generated incorrectly
    std::cout << "\nCastling rights: " << pos.castling_rights << std::endl;
    std::cout << "Black kingside: " << ((pos.castling_rights & 4) ? "available" : "unavailable") << std::endl;
    std::cout << "Black queenside: " << ((pos.castling_rights & 8) ? "available" : "unavailable") << std::endl;
    
    return 0;
}