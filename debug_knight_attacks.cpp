#include "src/bitboard_position.hpp"
#include "src/bitboard_attacks.hpp"
#include "src/bitboard.hpp"
#include <iostream>
#include <iomanip>
#include <string>

// Helper function to convert square to algebraic notation
std::string sq_to_alg(int square_64) {
    if (square_64 < 0 || square_64 >= 64) return "-";
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

// Test knight attack calculation for e4
int main() {
    std::cout << "=== Knight Attack Debug for e4 ===\n\n";

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

    // Get white knights
    uint64_t white_knights = pos.piece_bitboards[0][static_cast<int>(PieceType::Knight)];
    std::cout << "White knights bitboard: " << std::hex << (unsigned long long)white_knights << std::dec << "\n";

    // List all white knight positions
    std::cout << "\nWhite knight positions:\n";
    uint64_t knights_copy = white_knights;
    while (knights_copy) {
        int knight_sq = get_lsb(knights_copy);
        std::string algebraic = sq_to_alg(knight_sq);
        std::cout << "Knight at " << algebraic << " (square " << knight_sq << ")\n";

        // Show knight attacks
        uint64_t attacks = knight_attacks[knight_sq];
        std::cout << "  Attacks bitboard: " << std::hex << (unsigned long long)attacks << std::dec << "\n";

        // Check if it attacks e4 (square 28)
        uint64_t e4_bb = 1ULL << 28;
        bool attacks_e4 = (attacks & e4_bb) != 0;
        std::cout << "  Attacks e4: " << (attacks_e4 ? "YES" : "NO") << "\n";

        // List all squares this knight attacks
        std::cout << "  Attacks squares: ";
        for (int sq = 0; sq < 64; sq++) {
            if (attacks & (1ULL << sq)) {
                std::cout << sq_to_alg(sq) << " ";
            }
        }
        std::cout << "\n\n";

        knights_copy &= knights_copy - 1;
    }

    // Test the is_square_attacked function
    std::cout << "is_square_attacked(28, White): " << (pos.is_square_attacked(28, Color::White) ? "true" : "false") << "\n";

    return 0;
}