// src/main.cpp
#include <array>
#include <iostream>
#include <cassert>
#include <iomanip>

#include "chess_types.hpp"  // Piece, Color, PieceType, helpers
#include "board120.hpp"     // File, Rank, sq(), is_playable, etc.
#include "squares120.hpp" // MailboxMaps, ALL64, ALL120, etc.
#include "position.hpp"
#include "zobrist.hpp"
#include "init.hpp"         // Engine initialization
#include "board.hpp"        // Board management functions including print_position

int main() {
    // Initialize all engine subsystems
    Huginn::init();
    
    std::cout << "=== Huginn Chess Engine - Position Display Demo ===\n";
    
    // Create position and set up starting position
    Position pos;
    pos.set_startpos();
    
    std::cout << "\n1. Standard Chess Starting Position:\n";
    print_position(pos);
    
    std::cout << "FEN: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\n";
    
    // Show a more complex position (Kiwipete)
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "\n2. Kiwipete Test Position (complex position with castling possibilities):\n";
    
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    if (pos.set_from_fen(kiwipete_fen)) {
        print_position(pos);
        std::cout << "FEN: " << kiwipete_fen << "\n";
    }
    
    // Show a position with en passant
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "\n3. Position with En Passant (after 1.e4 e5 2.Nf3 Nc6 3.d4 exd4 4.Nxd4 Nf6 5.Nc3 d6 6.f4 Be7 7.Be2 O-O 8.O-O c5 9.Nb3 b6 10.Bf3 Bb7 11.Re1 Rc8 12.e5):\n";
    
    const std::string ep_fen = "2r1k2r/pb2bppp/1pn2n2/2ppP3/5P2/1NN1BQ2/PPP3PP/R1B1R1K1 b k - 0 12";
    if (pos.set_from_fen(ep_fen)) {
        print_position(pos);
        std::cout << "FEN: " << ep_fen << "\n";
    }
    
    // Show an endgame position
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "\n4. King and Pawn Endgame:\n";
    
    const std::string endgame_fen = "8/8/8/3k4/3P4/3K4/8/8 w - - 0 1";
    if (pos.set_from_fen(endgame_fen)) {
        print_position(pos);
        std::cout << "FEN: " << endgame_fen << "\n";
    }
    
    // Return to starting position and show zobrist
    pos.set_startpos();
    pos.zobrist_key = Zobrist::compute(pos);
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "\nStarting position Zobrist key: 0x" << std::hex << pos.zobrist_key << std::dec << "\n";
    
    std::cout << "\n=== Position display demo complete ===\n";
    
    return 0;
}
