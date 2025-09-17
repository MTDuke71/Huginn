#include "src/bitboard_perft_optimized.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

int main() {
    std::cout << "=== Analysis of h2h4 position ===\n";

    // Start from the initial position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Make the h2h4 move
    SimpleBitboardMove h2h4(63, 31); // h2 (63) to h4 (31) in 64-square notation
    // Wait, let me verify the square numbering - h2 should be 15, h4 should be 31
    SimpleBitboardMove h2h4_move(15, 31); // h2 (15) to h4 (31)

    std::cout << "Making move h2h4 (square " << h2h4_move.from_64 << " to " << h2h4_move.to_64 << ")\n";

    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(h2h4_move);

    std::cout << "Position after h2h4:\n";
    std::cout << "FEN: " << pos.get_fen() << "\n";

    // Generate all legal moves from this position
    BitboardMoveGen movegen;
    BitboardMoveList moves;
    movegen.generate_legal_moves(pos, moves);

    std::cout << "Legal moves from this position: " << moves.size() << "\n";
    std::cout << "Expected from debug output: 9330 (original method)\n";
    std::cout << "This suggests our debug numbering might be wrong\n";

    // Let's try a different square numbering for h2h4
    pos.unmake_move(h2h4_move, undo_info);  // Restore original position

    // Try h2=7*8+7=63, h4=3*8+7=31 (if 0-based from bottom-left)
    // But that's not right either. Let me print all squares to understand the numbering

    std::cout << "\nSquare contents from original position:\n";
    for (int sq = 0; sq < 64; sq++) {
        Piece piece = pos.piece_at(sq);
        if (piece != Piece::None) {
            int file = sq % 8;
            int rank = sq / 8;
            char file_char = 'a' + file;
            char rank_char = '1' + rank;
            std::cout << "Square " << sq << " (" << file_char << rank_char << "): ";

            // Print piece type
            PieceType pt = pos.piece_type_at(sq);
            switch (pt) {
                case PieceType::Pawn: std::cout << "Pawn"; break;
                case PieceType::Rook: std::cout << "Rook"; break;
                case PieceType::Knight: std::cout << "Knight"; break;
                case PieceType::Bishop: std::cout << "Bishop"; break;
                case PieceType::Queen: std::cout << "Queen"; break;
                case PieceType::King: std::cout << "King"; break;
                default: std::cout << "Unknown"; break;
            }

            Color color = pos.color_at(sq);
            std::cout << " (" << (color == Color::White ? "White" : "Black") << ")\n";
        }
    }

    return 0;
}