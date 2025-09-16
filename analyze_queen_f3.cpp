#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;

int main() {
    std::cout << "=== Queen f3 Move Analysis ===\n\n";
    
    // Kiwipete position
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Position: " << fen << "\n\n";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN!\n";
        return 1;
    }
    
    // Generate all legal moves
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    std::cout << "All Queen moves from f3 (square 21):\n";
    for (const auto& move : moves.moves) {
        if (move.from_64 == 21) {  // f3 = 21 (f=5, rank 3 = 2, so 2*8+5=21)
            char to_file = 'a' + (move.to_64 % 8);
            char to_rank = '1' + (move.to_64 / 8);
            std::cout << "  f3" << to_file << to_rank;
            if (move.is_capture) std::cout << " (capture)";
            std::cout << "\n";
        }
    }
    
    // Check specifically for f3f5
    bool found_f3f5 = false;
    for (const auto& move : moves.moves) {
        if (move.from_64 == 21 && move.to_64 == 37) {  // f5 = 37 (f=5, rank 5 = 4, so 4*8+5=37)
            found_f3f5 = true;
            break;
        }
    }
    
    std::cout << "\nf3f5 move found: " << (found_f3f5 ? "YES" : "NO") << "\n";
    
    if (!found_f3f5) {
        std::cout << "\n❌ CRITICAL: f3f5 move is missing!\n";
        std::cout << "This explains the -14,718 node discrepancy.\n";
        
        // Check what's on f5
        std::cout << "\nSquare f5 (37) contents: ";
        Piece piece_on_f5 = pos.piece_at(37);
        switch (piece_on_f5) {
            case Piece::Empty: std::cout << "Empty"; break;
            case Piece::WhitePawn: std::cout << "White Pawn"; break;
            case Piece::BlackPawn: std::cout << "Black Pawn"; break;
            case Piece::WhiteKnight: std::cout << "White Knight"; break;
            case Piece::BlackKnight: std::cout << "Black Knight"; break;
            case Piece::WhiteBishop: std::cout << "White Bishop"; break;
            case Piece::BlackBishop: std::cout << "Black Bishop"; break;
            case Piece::WhiteRook: std::cout << "White Rook"; break;
            case Piece::BlackRook: std::cout << "Black Rook"; break;
            case Piece::WhiteQueen: std::cout << "White Queen"; break;
            case Piece::BlackQueen: std::cout << "Black Queen"; break;
            case Piece::WhiteKing: std::cout << "White King"; break;
            case Piece::BlackKing: std::cout << "Black King"; break;
            default: std::cout << "Unknown"; break;
        }
        std::cout << "\n";
    }
    
    return 0;
}