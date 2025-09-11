// board.cpp
#include "board.hpp"
#include "position.hpp"
#include "chess_types.hpp"
#include "board120.hpp"
#include <cstring>
#include <iostream>
#include <iomanip>

// Print a chess position in a visual ASCII format
void print_position(const Position& pos) {
    std::cout << "\n   +---+---+---+---+---+---+---+---+\n";
    
    // Print from rank 8 down to rank 1 (index 7 down to 0)
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << " " << (rank + 1) << " |";
        
        for (int file = 0; file < 8; ++file) {
            int square = sq(static_cast<File>(file), static_cast<Rank>(rank));
            Piece piece = pos.at(square);
            char piece_char = to_char(piece);
            
            std::cout << " " << piece_char << " |";
        }
        
        std::cout << "\n   +---+---+---+---+---+---+---+---+\n";
    }
    
    std::cout << "     a   b   c   d   e   f   g   h\n\n";
    
    // Print game state information
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    std::cout << "Castling rights: ";
    if (pos.castling_rights == CASTLE_NONE) {
        std::cout << "-";
    } else {
        if (pos.castling_rights & CASTLE_WK) std::cout << "K";
        if (pos.castling_rights & CASTLE_WQ) std::cout << "Q";
        if (pos.castling_rights & CASTLE_BK) std::cout << "k";
        if (pos.castling_rights & CASTLE_BQ) std::cout << "q";
    }
    std::cout << "\n";
    
    std::cout << "En passant square: ";
    if (pos.ep_square == -1) {
        std::cout << "-";
    } else {
        char buf[3];
        to_algebraic(pos.ep_square, buf);
        std::cout << buf;
    }
    std::cout << "\n";
    
    std::cout << "Halfmove clock: " << pos.halfmove_clock << "\n";
    std::cout << "Fullmove number: " << pos.fullmove_number << "\n";
    
    // Print piece counts
    std::cout << "Piece counts: ";
    std::cout << "Pawns=" << pos.piece_counts[int(PieceType::Pawn)];
    std::cout << ", Rooks=" << pos.piece_counts[int(PieceType::Rook)];
    std::cout << ", Knights=" << pos.piece_counts[int(PieceType::Knight)];
    std::cout << ", Bishops=" << pos.piece_counts[int(PieceType::Bishop)];
    std::cout << ", Queens=" << pos.piece_counts[int(PieceType::Queen)];
    std::cout << ", Kings=" << pos.piece_counts[int(PieceType::King)] << "\n";
    
    // Print material scores
    std::cout << "Material: White=" << pos.get_material_score(Color::White);
    std::cout << ", Black=" << pos.get_material_score(Color::Black);
    std::cout << ", Balance=" << pos.get_material_balance() << "\n";
    
    std::cout << "Position key: 0x" << std::hex << pos.zobrist_key << std::dec << "\n";
}
