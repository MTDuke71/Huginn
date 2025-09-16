#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard.hpp"
#include <iostream>
#include <iomanip>
#include <intrin.h>

using namespace BitboardMoveGen;

// Detailed attack detection test
int main() {
    std::cout << "=== Detailed Attack Detection Test After f3f5 ===\n\n";

    // Kiwipete position
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Starting position: " << fen << "\n\n";

    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }

    // Make the f3f5 move
    SimpleBitboardMove f3f5;
    f3f5.from_64 = 21;  // f3
    f3f5.to_64 = 37;    // f5
    f3f5.is_capture = false;
    f3f5.is_ep_capture = false;
    f3f5.is_castling = false;
    f3f5.is_promotion = false;

    std::cout << "Making move f3f5...\n";
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(f3f5);

    std::cout << "\nAFTER f3f5:\n";
    std::cout << "Queen on f5 (37): " << (pos.piece_type_at(37) == PieceType::Queen ? "YES" : "NO") << "\n";

    // Test each piece type individually
    std::cout << "\n=== Testing Each Piece Type ===\n";

    int total_attacked = 0;

    // Test pawns
    int pawn_attacked = 0;
    uint64_t white_pawns = pos.piece_bitboards[0][static_cast<int>(PieceType::Pawn)];
    std::cout << "White pawns bitboard: " << std::hex << (unsigned long long)white_pawns << std::dec << "\n";
    std::cout << "Number of white pawns: " << __popcnt64(white_pawns) << "\n";
    for (int sq = 0; sq < 64; sq++) {
        if (pos.is_square_attacked(sq, Color::White)) {
            pawn_attacked++;
        }
    }
    std::cout << "Squares attacked by White: " << pawn_attacked << "/64\n";

    // Test a few specific squares manually for pawn attacks
    std::cout << "\n=== Manual Pawn Attack Tests ===\n";
    std::cout << "Square d3 (19): " << (pos.is_square_attacked(19, Color::White) ? "attacked" : "not attacked") << "\n";
    std::cout << "Square e3 (20): " << (pos.is_square_attacked(20, Color::White) ? "attacked" : "not attacked") << "\n";
    std::cout << "Square f3 (21): " << (pos.is_square_attacked(21, Color::White) ? "attacked" : "not attacked") << "\n";
    std::cout << "Square g3 (22): " << (pos.is_square_attacked(22, Color::White) ? "attacked" : "not attacked") << "\n";
    std::cout << "Square h3 (23): " << (pos.is_square_attacked(23, Color::White) ? "attacked" : "not attacked") << "\n";

    // Test squares that should be attacked by pawns
    std::cout << "\nSquares that should be attacked by White pawns:\n";
    std::cout << "Square c4 (26): " << (pos.is_square_attacked(26, Color::White) ? "attacked" : "not attacked") << "\n";
    std::cout << "Square d4 (27): " << (pos.is_square_attacked(27, Color::White) ? "attacked" : "not attacked") << "\n";
    std::cout << "Square e4 (28): " << (pos.is_square_attacked(28, Color::White) ? "attacked" : "not attacked") << "\n";
    std::cout << "Square f4 (29): " << (pos.is_square_attacked(29, Color::White) ? "attacked" : "not attacked") << "\n";
    std::cout << "Square g4 (30): " << (pos.is_square_attacked(30, Color::White) ? "attacked" : "not attacked") << "\n";

    // Unmake the move
    pos.unmake_move(f3f5, undo);

    return 0;
}