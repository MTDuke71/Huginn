/**
 * @brief Debug king move generation specifically
 */
#include <iostream>
#include <bitset>
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_move.hpp"
#include "../ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

std::string square_to_algebraic(int sq64) {
    char file = 'a' + (sq64 % 8);
    char rank = '1' + (sq64 / 8);
    return std::string(1, file) + std::string(1, rank);
}

std::string ultra_move_to_string(const UltraMove& move) {
    std::string result = square_to_algebraic(move.from()) + square_to_algebraic(move.to());
    if (move.is_capture()) result += " (capture)";
    if (move.is_castle()) result += " (castle)";
    return result;
}

void debug_king_attacks() {
    std::cout << "=== Debugging King Move Generation ===\n";
    
    UltraPosition pos;
    pos.set_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    
    std::cout << "Kiwipete position loaded.\n";
    std::cout << "White to move.\n";
    
    // Generate all moves to see the total
    UltraMoveList all_moves;
    int total_count = pos.generate_all_moves(all_moves);
    std::cout << "Total moves generated: " << total_count << " (should be 48)\n\n";
    
    // Count moves by type
    int king_moves = 0, castling_moves = 0, other_moves = 0;
    
    for (int i = 0; i < all_moves.size(); ++i) {
        const UltraMove& move = all_moves[i];
        
        // Check if this is a king move (from e1 = square 4)
        if (move.from() == 4) {
            if (move.is_castle()) {
                castling_moves++;
                std::cout << "Castling: " << ultra_move_to_string(move) << "\n";
            } else {
                king_moves++;
                std::cout << "King move: " << ultra_move_to_string(move) << "\n";
            }
        } else {
            other_moves++;
        }
    }
    
    std::cout << "\nMove breakdown:\n";
    std::cout << "Regular king moves: " << king_moves << "\n";
    std::cout << "Castling moves: " << castling_moves << "\n";
    std::cout << "Other moves: " << other_moves << "\n";
    std::cout << "Total: " << (king_moves + castling_moves + other_moves) << "\n";
    
    // Test the king attack pattern directly
    std::cout << "\n=== Testing King Attack Pattern ===\n";
    
    int king_square = 4; // e1
    uint64_t king_attacks = UltraAttacks::king(king_square);
    
    std::cout << "King on e1 (square " << king_square << ")\n";
    std::cout << "King attack pattern: " << std::bitset<64>(king_attacks) << "\n";
    std::cout << "King attack squares:\n";
    
    for (int sq = 0; sq < 64; ++sq) {
        if (king_attacks & (1ULL << sq)) {
            std::cout << "  " << square_to_algebraic(sq) << " (square " << sq << ")\n";
        }
    }
    
    // Check what's blocking the king
    std::cout << "\n=== Checking King Move Legality ===\n";
    
    // We'd need to access the position's internal state to debug this properly
    // For now, let's just show what should be possible
    
    std::cout << "Expected king moves from e1:\n";
    std::cout << "- d1 (if not occupied by own piece)\n";
    std::cout << "- d2 (if not occupied by own piece and not attacked)\n";
    std::cout << "- e2 (if not occupied by own piece and not attacked)\n";
    std::cout << "- f1 (if not occupied by own piece and not attacked)\n";
    std::cout << "- f2 (if not occupied by own piece and not attacked)\n";
    std::cout << "\nNote: Castling moves are separate and already working correctly.\n";
}

int main() {
    try {
        UltraAttacks::initialize();
        std::cout << "UltraEngine initialized successfully.\n\n";
        
        debug_king_attacks();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}