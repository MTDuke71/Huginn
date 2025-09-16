#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard.hpp"
#include <iostream>
#include <iomanip>
#include <intrin.h>

using namespace BitboardMoveGen;

// Test occupied bitboard after f3f5
int main() {
    std::cout << "=== Occupied Bitboard Test After f3f5 ===\n\n";

    // Kiwipete position
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Starting position: " << fen << "\n\n";

    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }

    std::cout << "BEFORE f3f5:\n";
    std::cout << "Occupied bitboard: " << std::hex << (unsigned long long)pos.occupied_bitboard << std::dec << "\n";
    std::cout << "White pieces: " << std::hex << (unsigned long long)pos.color_bitboards[0] << std::dec << "\n";
    std::cout << "Black pieces: " << std::hex << (unsigned long long)pos.color_bitboards[1] << std::dec << "\n";

    // Make the f3f5 move
    SimpleBitboardMove f3f5;
    f3f5.from_64 = 21;  // f3
    f3f5.to_64 = 37;    // f5
    f3f5.is_capture = false;
    f3f5.is_ep_capture = false;
    f3f5.is_castling = false;
    f3f5.is_promotion = false;

    std::cout << "\nMaking move f3f5...\n";
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(f3f5);

    std::cout << "\nAFTER f3f5:\n";
    std::cout << "Occupied bitboard: " << std::hex << (unsigned long long)pos.occupied_bitboard << std::dec << "\n";
    std::cout << "White pieces: " << std::hex << (unsigned long long)pos.color_bitboards[0] << std::dec << "\n";
    std::cout << "Black pieces: " << std::hex << (unsigned long long)pos.color_bitboards[1] << std::dec << "\n";

    // Check specific squares
    std::cout << "\nSquare contents:\n";
    std::cout << "f3 (21): " << (pos.piece_at(21) == Piece::None ? "empty" : "occupied") << "\n";
    std::cout << "f5 (37): " << (pos.piece_at(37) == Piece::None ? "empty" : "occupied") << " - " << static_cast<int>(pos.piece_type_at(37)) << "\n";

    // Test bishop attacks from f5
    std::cout << "\nBishop attacks from f5 (37):\n";
    uint64_t bishop_attacks_f5 = 0;
    if (pos.piece_type_at(37) == PieceType::Queen) {  // Queen can move like bishop
        bishop_attacks_f5 = bishop_attacks(37, pos.occupied_bitboard);
        std::cout << "Bishop attacks: " << std::hex << (unsigned long long)bishop_attacks_f5 << std::dec << "\n";
        std::cout << "Number of attacked squares: " << __popcnt64(bishop_attacks_f5) << "\n";
    }

    // Test if any square is attacked by the queen on f5
    std::cout << "\nTesting attacks from queen on f5:\n";
    int attacked_count = 0;
    for (int sq = 0; sq < 64; sq++) {
        if (pos.is_square_attacked(sq, Color::White)) {
            attacked_count++;
        }
    }
    std::cout << "Total squares attacked by White: " << attacked_count << "/64\n";

    // Unmake the move
    pos.unmake_move(f3f5, undo);

    return 0;
}