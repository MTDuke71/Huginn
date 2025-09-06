// Debug the mirror test failure for position #3
#include "src/position.hpp"
#include "src/minimal_search.hpp"
#include "src/init.hpp"
#include <iostream>

using namespace Huginn;

void print_position_board(const Position& pos) {
    std::cout << "Board layout:" << std::endl;
    for (int rank = 7; rank >= 0; --rank) {
        std::cout << (rank + 1) << " ";
        for (int file = 0; file < 8; ++file) {
            int sq120 = 21 + rank * 10 + file;
            Piece piece = pos.board[sq120];
            char c = '.';
            if (piece != Piece::None) {
                PieceType type = type_of(piece);
                Color color = color_of(piece);
                char piece_char = ' ';
                switch(type) {
                    case PieceType::Pawn: piece_char = 'P'; break;
                    case PieceType::Knight: piece_char = 'N'; break;
                    case PieceType::Bishop: piece_char = 'B'; break;
                    case PieceType::Rook: piece_char = 'R'; break;
                    case PieceType::Queen: piece_char = 'Q'; break;
                    case PieceType::King: piece_char = 'K'; break;
                    default: piece_char = '?'; break;
                }
                c = (color == Color::White) ? piece_char : (piece_char + 32);
            }
            std::cout << c << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "  a b c d e f g h" << std::endl;
}

void debug_position_detailed(const std::string& description, const std::string& fen, MinimalEngine& engine) {
    std::cout << "\n=== " << description << " ===" << std::endl;
    std::cout << "FEN: " << fen << std::endl;
    
    Position pos;
    pos.set_from_fen(fen);
    
    std::cout << "\nOriginal Position:" << std::endl;
    print_position_board(pos);
    
    // Evaluate original
    int eval1 = engine.evalPosition(pos);
    std::cout << "Original eval: " << eval1 << " cp" << std::endl;
    
    // Create and evaluate mirrored
    Position mirrored = engine.mirrorBoard(pos);
    int eval2 = engine.evalPosition(mirrored);
    
    std::cout << "\nMirrored Position:" << std::endl;
    print_position_board(mirrored);
    std::cout << "Mirrored eval: " << eval2 << " cp" << std::endl;
    
    // Show the difference
    int diff = eval1 - eval2;
    std::cout << "\nDifference: " << diff << " cp" << std::endl;
    
    // Get mirrored FEN for inspection
    std::string mirrored_fen = mirrored.to_fen();
    std::cout << "Mirrored FEN: " << mirrored_fen << std::endl;
}

int main() {
    std::cout << "=== Debug Mirror Test Failure ===" << std::endl;
    
    init();
    MinimalEngine engine;
    
    // Test the failing position
    debug_position_detailed("Failing Position #3", 
                           "r3k2r/pp3ppp/2n5/2bp4/2nP4/2P5/PP3PPP/R3K2R w KQkq - 0 10",
                           engine);
    
    return 0;
}
