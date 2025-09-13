/**
 * @file board.cpp
 * @brief Chess board display and visualization utilities
 * 
 * Provides functionality for displaying chess positions in human-readable ASCII format.
 * The board display follows standard chess notation with ranks 1-8 and files a-h,
 * showing pieces using standard algebraic notation characters.
 * 
 * ## Display Format
 * - White pieces: UPPERCASE (K, Q, R, B, N, P)
 * - Black pieces: lowercase (k, q, r, b, n, p)  
 * - Empty squares: '.' character
 * - Board borders: ASCII art with rank/file labels
 * 
 * ## Functions
 * - print_position(): Displays a position in visual ASCII format
 * - Side-to-move, castling rights, and en passant status included
 * 
 * @author MTDuke71
 * @version 1.2
 * @see position.hpp for Position structure
 */
#include "board.hpp"
#include "position.hpp"
#include "chess_types.hpp"
#include "board120.hpp"
#include <cstring>
#include <iostream>
#include <iomanip>

/**
 * @brief Print a chess position in visual ASCII format
 * 
 * Displays the chess board from White's perspective (rank 8 at top, rank 1 at bottom)
 * with clear borders and coordinate labels. Shows piece positions using standard
 * algebraic notation characters.
 * 
 * @param pos The chess position to display
 * 
 * Example output:
 * ```
 *    +---+---+---+---+---+---+---+---+
 *  8 | r | n | b | q | k | b | n | r |
 *    +---+---+---+---+---+---+---+---+
 *  7 | p | p | p | p | p | p | p | p |
 *    +---+---+---+---+---+---+---+---+
 *  ...
 *      a   b   c   d   e   f   g   h
 * ```
 */
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
