/**
 * Position Analysis: Mirror Board Investigation
 * 
 * This analyzes the exact issue with the mirrorBoard function.
 * The problem is that it mirrors the current piece placement but doesn't
 * ensure the resulting position represents a valid mirrored game.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include <iostream>

void analyze_mirror_issue() {
    using namespace Huginn;
    
    std::cout << "Mirror Board Issue Analysis\n";
    std::cout << "===========================\n\n";
    
    // King's Gambit: 1.e4 e5 2.f4
    std::string original_fen = "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2";
    Position original_pos;
    original_pos.set_from_fen(original_fen);
    
    std::cout << "Original King's Gambit Position:\n";
    std::cout << "================================\n";
    std::cout << "FEN: " << original_fen << "\n";
    std::cout << "White pawns: a2, b2, c2, d2, e4, f4, g2, h2 (f2→f4)\n";
    std::cout << "Black pawns: a7, b7, c7, d7, e5, f7, g7, h7 (e7→e5)\n\n";
    
    MinimalEngine engine;
    Position mirrored_pos = engine.mirrorBoard(original_pos);
    std::string mirrored_fen = mirrored_pos.to_fen();
    
    std::cout << "Mirrored Position (by mirrorBoard function):\n";
    std::cout << "============================================\n";
    std::cout << "FEN: " << mirrored_fen << "\n";
    
    // Analyze the mirrored position pawn structure
    std::cout << "Analysis of mirrored pawn placement:\n";
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int sq64 = rank * 8 + file;
            int sq120 = MAILBOX_MAPS.to120[sq64];
            if (sq120 >= 0) {
                Piece piece = mirrored_pos.board[sq120];
                if (piece == Piece::WhitePawn || piece == Piece::BlackPawn) {
                    char file_char = 'a' + file;
                    char rank_char = '1' + rank;
                    char piece_char = (piece == Piece::WhitePawn) ? 'P' : 'p';
                    std::cout << piece_char << " on " << file_char << rank_char << "\n";
                }
            }
        }
    }
    
    std::cout << "\nPROBLEM IDENTIFIED:\n";
    std::cout << "===================\n";
    std::cout << "The mirrorBoard function correctly mirrors piece positions,\n";
    std::cout << "but it produces a position that's not equivalent to mirroring\n";
    std::cout << "the game moves that led to the original position.\n\n";
    
    std::cout << "Expected mirrored game: 1.e4 e5 2.f5 (black plays f7-f5)\n";
    std::cout << "This should result in:\n";
    std::cout << "- White pawns: a2, b2, c2, d2, e4, f2, g2, h2\n";
    std::cout << "- Black pawns: a7, b7, c7, d7, e5, f5, g7, h7\n\n";
    
    std::cout << "But mirrorBoard creates:\n";
    std::cout << "- A position where pieces are mirrored from current state\n";
    std::cout << "- This doesn't preserve the move history symmetry\n\n";
    
    std::cout << "SOLUTION:\n";
    std::cout << "=========\n";
    std::cout << "For evaluation symmetry testing, we need to either:\n";
    std::cout << "1. Test from symmetric starting positions\n";
    std::cout << "2. Use positions where move history creates true symmetry\n";
    std::cout << "3. Account for the evaluation perspective correctly\n";
}

int main() {
    Huginn::init();
    analyze_mirror_issue();
    return 0;
}
