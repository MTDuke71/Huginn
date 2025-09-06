/**
 * Mirror Board Demo - VICE Tutorial Implementation
 * 
 * Demonstrates the mirrorBoard function for evaluation symmetry testing.
 * This is based on the VICE tutorial approach for detecting asymmetrical
 * evaluation errors by creating mirrored positions.
 * 
 * VICE Tutorial Concept:
 * - Mirror the board vertically (rank 1 <-> rank 8)
 * - Swap piece colors (White <-> Black)
 * - Flip side to move
 * - Mirror castling permissions
 * - Mirror en passant square
 * 
 * This allows testing that evaluation(position) == evaluation(mirror(position))
 * which helps detect asymmetrical bugs in the evaluation function.
 */

#include "../src/position.hpp"
#include "../src/minimal_search.hpp"
#include "../src/init.hpp"
#include "../src/input_checking.hpp"
#include "../src/board120.hpp"
#include "../src/chess_types.hpp"
#include <iostream>
#include <iomanip>
#include <string>

// MSVC compatibility for popcount
#ifdef _MSC_VER
#include <intrin.h>
inline int popcount(uint64_t x) { return (int)__popcnt64(x); }
#else
inline int popcount(uint64_t x) { return __builtin_popcountll(x); }
#endif

void print_board_with_labels(const Position& pos, const std::string& title) {
    using namespace Huginn;
    
    std::cout << "\n" << title << ":\n";
    std::cout << "   +---+---+---+---+---+---+---+---+\n";
    
    for (int rank = 8; rank >= 1; --rank) {
        std::cout << " " << rank << " |";
        for (int file = 1; file <= 8; ++file) {
            // Convert 1-based file/rank to 0-based and then to square120
            int sq120 = sq(File(file - 1), Rank(rank - 1));
            Piece piece = pos.board[sq120];
            
            char piece_char = ' ';
            switch (piece) {
                case Piece::WhitePawn:   piece_char = 'P'; break;
                case Piece::WhiteKnight: piece_char = 'N'; break;
                case Piece::WhiteBishop: piece_char = 'B'; break;
                case Piece::WhiteRook:   piece_char = 'R'; break;
                case Piece::WhiteQueen:  piece_char = 'Q'; break;
                case Piece::WhiteKing:   piece_char = 'K'; break;
                case Piece::BlackPawn:   piece_char = 'p'; break;
                case Piece::BlackKnight: piece_char = 'n'; break;
                case Piece::BlackBishop: piece_char = 'b'; break;
                case Piece::BlackRook:   piece_char = 'r'; break;
                case Piece::BlackQueen:  piece_char = 'q'; break;
                case Piece::BlackKing:   piece_char = 'k'; break;
                default: piece_char = ' '; break;
            }
            
            std::cout << " " << piece_char << " |";
        }
        std::cout << "\n   +---+---+---+---+---+---+---+---+\n";
    }
    
    std::cout << "     a   b   c   d   e   f   g   h\n";
    
    // Print position details
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    
    std::cout << "Castling: ";
    if (pos.castling_rights & CASTLE_WK) std::cout << "K";
    if (pos.castling_rights & CASTLE_WQ) std::cout << "Q";
    if (pos.castling_rights & CASTLE_BK) std::cout << "k";
    if (pos.castling_rights & CASTLE_BQ) std::cout << "q";
    if (pos.castling_rights == 0) std::cout << "-";
    std::cout << "\n";
    
    std::cout << "En passant: ";
    if (pos.ep_square >= 0) {
        char alg[3];
        const char* alg_str = to_algebraic(pos.ep_square, alg);
        std::cout << alg_str;
    } else {
        std::cout << "-";
    }
    std::cout << "\n";
    
    std::cout << "Halfmove clock: " << pos.halfmove_clock << "\n";
    std::cout << "Fullmove: " << pos.fullmove_number << "\n";
}

void demonstrate_mirror_board(const std::string& fen, const std::string& description) {
    using namespace Huginn;
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Mirror Board Test: " << description << "\n";
    std::cout << "FEN: " << fen << "\n";
    std::cout << std::string(60, '=') << "\n";
    
    Position pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "Error: Invalid FEN!\n";
        return;
    }
    
    // Print original position
    print_board_with_labels(pos, "Original Position");
    
    // Create mirrored position
    MinimalEngine engine;
    Position mirrored = engine.mirrorBoard(pos);
    
    // Print mirrored position
    print_board_with_labels(mirrored, "Mirrored Position");
    
    // Test evaluation symmetry
    int original_eval = engine.evaluate(pos);
    int mirrored_eval = engine.evaluate(mirrored);
    
    std::cout << "\nEvaluation Test:\n";
    std::cout << "Original evaluation: " << std::setw(6) << original_eval << "\n";
    std::cout << "Mirrored evaluation: " << std::setw(6) << mirrored_eval << "\n";
    std::cout << "Difference: " << std::setw(6) << (original_eval - mirrored_eval) << "\n";
    
    if (original_eval == mirrored_eval) {
        std::cout << "✓ PASS: Evaluation is symmetrical!\n";
    } else {
        std::cout << "✗ FAIL: Evaluation asymmetry detected!\n";
        std::cout << "This indicates a potential bug in the evaluation function.\n";
    }
    
    // Print FEN of mirrored position for verification
    std::cout << "\nMirrored FEN: " << mirrored.to_fen() << "\n";
}

int main() {
    std::cout << "Mirror Board Demo - VICE Tutorial Implementation\n";
    std::cout << "================================================\n\n";
    std::cout << "This demo shows the mirrorBoard function that creates mirrored\n";
    std::cout << "positions for evaluation symmetry testing (VICE tutorial concept).\n";
    std::cout << "\nThe function:\n";
    std::cout << "• Mirrors the board vertically (rank 1 ↔ rank 8)\n";
    std::cout << "• Swaps all piece colors (White ↔ Black)\n";
    std::cout << "• Flips the side to move\n";
    std::cout << "• Mirrors castling permissions\n";
    std::cout << "• Mirrors en passant square\n";
    std::cout << "\nA good evaluation function should give the same score\n";
    std::cout << "for a position and its mirror (symmetry test).\n";
    
    // Initialize the engine
    Huginn::init();
    
    
    // Test cases
    
    // Starting position
    demonstrate_mirror_board(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "Starting Position"
    );
    
    // Sicilian Defense position
    demonstrate_mirror_board(
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
        "Sicilian Defense (after 1.e4 c5)"
    );
    
    // King's Gambit position with castling rights
    demonstrate_mirror_board(
        "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2",
        "King's Gambit (after 1.e4 e5 2.f4)"
    );
    
    // Endgame position
    demonstrate_mirror_board(
        "8/8/8/3k4/3P4/3K4/8/8 w - - 0 1",
        "Simple King and Pawn Endgame"
    );
    
    // Position with en passant
    demonstrate_mirror_board(
        "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
        "Position with En Passant Square"
    );
    
    // Asymmetrical position to test evaluation
    demonstrate_mirror_board(
        "rnbqkb1r/pppppppp/5n2/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 4 3",
        "Asymmetrical Knight Development"
    );
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Mirror Board Demo Complete!\n";
    std::cout << "\nThe mirrorBoard function is useful for:\n";
    std::cout << "• Testing evaluation function symmetry\n";
    std::cout << "• Detecting asymmetrical bugs\n";
    std::cout << "• Validating piece-square table implementations\n";
    std::cout << "• VICE tutorial Part 78 evaluation testing\n";
    std::cout << std::string(60, '=') << "\n";
    
    return 0;
}
