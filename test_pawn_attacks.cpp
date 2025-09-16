#include "src/bitboard_position.hpp"
#include "src/bitboard_attacks.hpp"
#include "src/bitboard.hpp"
#include <iostream>
#include <iomanip>
#include <intrin.h>

// Test pawn attack calculation
int main() {
    std::cout << "=== Pawn Attack Calculation Test ===\n\n";

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

    // Get white pawns
    uint64_t white_pawns = pos.piece_bitboards[0][static_cast<int>(PieceType::Pawn)];
    std::cout << "White pawns bitboard: " << std::hex << (unsigned long long)white_pawns << std::dec << "\n";

    // Calculate pawn attacks manually
    uint64_t pawn_attacks = ((white_pawns & ~FILE_A_BB) << 7) | ((white_pawns & ~FILE_H_BB) << 9);
    std::cout << "Calculated pawn attacks: " << std::hex << (unsigned long long)pawn_attacks << std::dec << "\n";
    std::cout << "Number of squares attacked by pawns: " << __popcnt64(pawn_attacks) << "\n";

    // Test specific squares
    std::cout << "\nTesting specific squares:\n";
    for (int sq = 0; sq < 64; sq++) {
        uint64_t target_bb = 1ULL << sq;
        if (pawn_attacks & target_bb) {
            std::cout << "Square " << sq << " attacked by pawn\n";
        }
    }

    // Test the is_square_attacked function for a specific square
    std::cout << "\nTesting is_square_attacked for square 28 (e4):\n";
    bool attacked = pos.is_square_attacked(28, Color::White);
    std::cout << "is_square_attacked(28, White): " << (attacked ? "true" : "false") << "\n";

    // Check if e4 should be attacked by pawns
    uint64_t e4_bb = 1ULL << 28;
    bool should_be_attacked_by_pawn = (pawn_attacks & e4_bb) != 0;
    std::cout << "Should be attacked by pawn: " << (should_be_attacked_by_pawn ? "true" : "false") << "\n";

    // Test each piece type individually for square 28
    std::cout << "\n=== Testing Each Piece Type for Square 28 (e4) ===\n";

    // Pawns
    uint64_t enemy_pawns = pos.piece_bitboards[0][static_cast<int>(PieceType::Pawn)];
    uint64_t pawn_attack_bb = ((enemy_pawns & ~FILE_A_BB) << 7) | ((enemy_pawns & ~FILE_H_BB) << 9);
    bool pawn_attacks_e4 = (pawn_attack_bb & e4_bb) != 0;
    std::cout << "Pawn attacks e4: " << (pawn_attacks_e4 ? "true" : "false") << "\n";

    // Knights
    uint64_t enemy_knights = pos.piece_bitboards[0][static_cast<int>(PieceType::Knight)];
    bool knight_attacks_e4 = false;
    while (enemy_knights) {
        int knight_sq = get_lsb(enemy_knights);
        if (knight_attacks[knight_sq] & e4_bb) {
            knight_attacks_e4 = true;
            break;
        }
        enemy_knights &= enemy_knights - 1;
    }
    std::cout << "Knight attacks e4: " << (knight_attacks_e4 ? "true" : "false") << "\n";

    // Bishops/Queens
    uint64_t enemy_bishops_queens = pos.piece_bitboards[0][static_cast<int>(PieceType::Bishop)] |
                                   pos.piece_bitboards[0][static_cast<int>(PieceType::Queen)];
    bool bishop_queen_attacks_e4 = false;
    while (enemy_bishops_queens) {
        int piece_sq = get_lsb(enemy_bishops_queens);
        uint64_t attacks = bishop_attacks(piece_sq, pos.occupied_bitboard);
        if (attacks & e4_bb) {
            bishop_queen_attacks_e4 = true;
            break;
        }
        enemy_bishops_queens &= enemy_bishops_queens - 1;
    }
    std::cout << "Bishop/Queen attacks e4: " << (bishop_queen_attacks_e4 ? "true" : "false") << "\n";

    // Rooks/Queens
    uint64_t enemy_rooks_queens = pos.piece_bitboards[0][static_cast<int>(PieceType::Rook)] |
                                 pos.piece_bitboards[0][static_cast<int>(PieceType::Queen)];
    bool rook_queen_attacks_e4 = false;
    while (enemy_rooks_queens) {
        int piece_sq = get_lsb(enemy_rooks_queens);
        uint64_t attacks = rook_attacks(piece_sq, pos.occupied_bitboard);
        if (attacks & e4_bb) {
            rook_queen_attacks_e4 = true;
            break;
        }
        enemy_rooks_queens &= enemy_rooks_queens - 1;
    }
    std::cout << "Rook/Queen attacks e4: " << (rook_queen_attacks_e4 ? "true" : "false") << "\n";

    // King
    uint64_t enemy_king = pos.piece_bitboards[0][static_cast<int>(PieceType::King)];
    bool king_attacks_e4 = false;
    if (enemy_king) {
        int king_sq = get_lsb(enemy_king);
        if (king_attacks[king_sq] & e4_bb) {
            king_attacks_e4 = true;
        }
    }
    std::cout << "King attacks e4: " << (king_attacks_e4 ? "true" : "false") << "\n";

    std::cout << "\nSummary: e4 is attacked by White: " << (attacked ? "YES" : "NO") << "\n";

    return 0;
}