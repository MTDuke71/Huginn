#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

using namespace BitboardMoveGen;

// Test attack detection after f3f5
int main() {
    std::cout << "=== Attack Detection Test After f3f5 ===\n\n";

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

    // Check position after f3f5
    std::string after_fen = pos.to_fen();
    std::cout << "Position after f3f5: " << after_fen << "\n\n";

    // Check if kings are in check
    int white_king_sq = pos.king_square_64[0];
    int black_king_sq = pos.king_square_64[1];
    bool white_king_in_check = pos.is_square_attacked(white_king_sq, Color::Black);
    bool black_king_in_check = pos.is_square_attacked(black_king_sq, Color::White);

    std::cout << "White king at " << white_king_sq << " - in check: " << (white_king_in_check ? "YES" : "NO") << "\n";
    std::cout << "Black king at " << black_king_sq << " - in check: " << (black_king_in_check ? "YES" : "NO") << "\n\n";

    // Test a few specific squares to see if attack detection is working
    std::cout << "Testing attack detection on specific squares:\n";

    // Test squares around the black king
    int test_squares[] = {60, 61, 62, 59, 58, 52, 53, 54};
    const char* square_names[] = {"e8", "f8", "g8", "d8", "c8", "e7", "f7", "g7"};

    for (int i = 0; i < 8; i++) {
        int sq = test_squares[i];
        bool attacked_by_white = pos.is_square_attacked(sq, Color::White);
        bool attacked_by_black = pos.is_square_attacked(sq, Color::Black);
        std::cout << "Square " << square_names[i] << " (" << sq << "): attacked by White=" << (attacked_by_white ? "YES" : "NO")
                  << ", attacked by Black=" << (attacked_by_black ? "YES" : "NO") << "\n";
    }

    // Test some white piece attacks
    std::cout << "\nTesting attacks from white pieces:\n";
    int white_piece_squares[] = {4, 5, 12, 13, 18, 21};  // e1, f1, e2, f2, c3, f3
    const char* piece_names[] = {"e1(king)", "f1", "e2", "f2", "c3(knight)", "f3(queen)"};

    for (int i = 0; i < 6; i++) {
        int sq = white_piece_squares[i];
        Piece piece = pos.piece_at(sq);
        if (piece != Piece::None) {
            // Test what squares this piece attacks
            std::cout << piece_names[i] << " attacks: ";
            for (int target = 0; target < 64; target++) {
                if (pos.is_square_attacked(target, Color::White)) {
                    char file = 'a' + (target % 8);
                    char rank = '1' + (target / 8);
                    std::cout << file << rank << " ";
                }
            }
            std::cout << "\n";
        }
    }

    // Unmake the move
    pos.unmake_move(f3f5, undo);

    return 0;
}