#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard.hpp"  // For pop_lsb and other bitboard utilities
#include <iostream>

using namespace BitboardMoveGen;

std::string square_to_algebraic_local(int square_64) {
    if (square_64 < 0 || square_64 >= 64) return "-";
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

void analyze_position_differences() {
    std::cout << "=== Position Analysis: Starting vs Kiwipete ===" << std::endl;

    // Starting position
    BitboardPosition start_pos;
    start_pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Kiwipete position
    BitboardPosition kiwi_pos;
    kiwi_pos.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    std::cout << "\n--- Piece Differences ---" << std::endl;

    // Compare piece counts
    for (int color = 0; color < 2; ++color) {
        Color c = static_cast<Color>(color);
        std::string color_name = (c == Color::White) ? "White" : "Black";

        std::cout << color_name << " pieces:" << std::endl;

        for (int piece_type = 1; piece_type < 7; ++piece_type) {
            PieceType pt = static_cast<PieceType>(piece_type);

            uint64_t start_pieces = start_pos.get_pieces(c, pt);
            uint64_t kiwi_pieces = kiwi_pos.get_pieces(c, pt);

            int start_count = popcount(start_pieces);
            int kiwi_count = popcount(kiwi_pieces);

            const char* piece_names[] = {"", "Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};

            std::cout << "  " << piece_names[piece_type] << ": Start=" << start_count
                      << " Kiwi=" << kiwi_count;
            if (start_count != kiwi_count) {
                std::cout << " *** DIFFERENT ***";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "--- Special Features in Kiwipete ---" << std::endl;

    // Check for en passant
    std::cout << "En passant square: " << kiwi_pos.ep_square_64 << std::endl;

    // Check castling rights
    std::cout << "Castling rights: " << kiwi_pos.castling_rights << std::endl;

    // Check for advanced piece positions
    std::cout << "\nAdvanced piece positions in Kiwipete:" << std::endl;

    // Find white knights
    uint64_t white_knights = kiwi_pos.get_pieces(Color::White, PieceType::Knight);
    std::cout << "White knights: ";
    while (white_knights) {
        int sq = get_lsb(white_knights);
        std::cout << square_to_algebraic_local(sq) << " ";
        white_knights &= white_knights - 1;
    }
    std::cout << std::endl;

    // Find white queen
    uint64_t white_queen = kiwi_pos.get_pieces(Color::White, PieceType::Queen);
    std::cout << "White queen: ";
    while (white_queen) {
        int sq = get_lsb(white_queen);
        std::cout << square_to_algebraic_local(sq) << " ";
        white_queen &= white_queen - 1;
    }
    std::cout << std::endl;
}

int main() {
    analyze_position_differences();
    return 0;
}