/**
 * Debug: Check Exact Pawn Positions
 * 
 * Debug tool to see exactly where pawns are positioned in both FENs
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include <iostream>

void debug_pawn_positions() {
    using namespace Huginn;
    
    std::cout << "Debug: Exact Pawn Positions\n";
    std::cout << "============================\n\n";
    
    // Position after 1.e4 c6
    std::string e4_c6_fen = "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";
    Position original_pos;
    original_pos.set_from_fen(e4_c6_fen);
    
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    
    std::cout << "Original Position Pawns:\n";
    std::cout << "========================\n";
    for (int sq120 = 21; sq120 <= 98; ++sq120) {
        if (original_pos.board[sq120] == Piece::WhitePawn || original_pos.board[sq120] == Piece::BlackPawn) {
            int sq64 = MAILBOX_MAPS.to64[sq120];
            int file = sq64 % 8;
            int rank = sq64 / 8;
            char file_char = 'a' + file;
            char rank_char = '1' + rank;
            char piece_char = (original_pos.board[sq120] == Piece::WhitePawn) ? 'P' : 'p';
            
            std::cout << piece_char << " on " << file_char << rank_char 
                      << " (sq120=" << sq120 << ", sq64=" << sq64 << ")\n";
        }
    }
    
    std::cout << "\nMirrored Position Pawns:\n";
    std::cout << "========================\n";
    for (int sq120 = 21; sq120 <= 98; ++sq120) {
        if (mirrored_pos.board[sq120] == Piece::WhitePawn || mirrored_pos.board[sq120] == Piece::BlackPawn) {
            int sq64 = MAILBOX_MAPS.to64[sq120];
            int file = sq64 % 8;
            int rank = sq64 / 8;
            char file_char = 'a' + file;
            char rank_char = '1' + rank;
            char piece_char = (mirrored_pos.board[sq120] == Piece::WhitePawn) ? 'P' : 'p';
            
            std::cout << piece_char << " on " << file_char << rank_char 
                      << " (sq120=" << sq120 << ", sq64=" << sq64 << ")\n";
        }
    }
    
    std::cout << "\nCenter Squares Check:\n";
    std::cout << "=====================\n";
    
    // Check specific center squares
    std::cout << "e4 (sq64=28): ";
    int e4_sq120 = MAILBOX_MAPS.to120[28];
    std::cout << "sq120=" << e4_sq120 << ", piece=";
    if (original_pos.board[e4_sq120] == Piece::WhitePawn) std::cout << "WhitePawn";
    else if (original_pos.board[e4_sq120] == Piece::BlackPawn) std::cout << "BlackPawn";
    else if (original_pos.board[e4_sq120] == Piece::None) std::cout << "Empty";
    else std::cout << "Other";
    std::cout << "\n";
    
    std::cout << "e5 (sq64=36): ";
    int e5_sq120 = MAILBOX_MAPS.to120[36];
    std::cout << "sq120=" << e5_sq120 << ", piece=";
    if (mirrored_pos.board[e5_sq120] == Piece::WhitePawn) std::cout << "WhitePawn";
    else if (mirrored_pos.board[e5_sq120] == Piece::BlackPawn) std::cout << "BlackPawn";
    else if (mirrored_pos.board[e5_sq120] == Piece::None) std::cout << "Empty";
    else std::cout << "Other";
    std::cout << "\n";
}

int main() {
    Huginn::init();
    debug_pawn_positions();
    return 0;
}
