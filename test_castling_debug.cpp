/**
 * @brief Test castling rights parsing and generation
 */
#include <iostream>
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
    if (move.is_castle()) result += " (castle)";
    return result;
}

void test_castling() {
    std::cout << "=== Testing Castling Generation ===\n";
    
    // Test starting position first (should have 2 castling moves)
    std::cout << "\n--- Starting Position ---\n";
    UltraPosition start_pos;
    start_pos.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    UltraMoveList start_moves;
    int start_count = start_pos.generate_all_moves(start_moves);
    std::cout << "Starting position total moves: " << start_count << " (should be 20)\n";
    
    int start_castling = 0;
    for (int i = 0; i < start_moves.size(); ++i) {
        if (start_moves[i].is_castle()) {
            std::cout << "  Castling move: " << ultra_move_to_string(start_moves[i]) << "\n";
            start_castling++;
        }
    }
    std::cout << "Castling moves in starting position: " << start_castling << " (should be 0 - blocked by pieces)\n";
    
    // Test open castling position
    std::cout << "\n--- Open Castling Position ---\n";
    UltraPosition castle_pos;
    castle_pos.set_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    
    UltraMoveList castle_moves;
    int castle_count = castle_pos.generate_all_moves(castle_moves);
    std::cout << "Open castling position total moves: " << castle_count << "\n";
    
    int castling_moves = 0;
    for (int i = 0; i < castle_moves.size(); ++i) {
        if (castle_moves[i].is_castle()) {
            std::cout << "  Castling move: " << ultra_move_to_string(castle_moves[i]) << "\n";
            castling_moves++;
        }
    }
    std::cout << "Castling moves: " << castling_moves << " (should be 2)\n";
    
    // Test Kiwipete position  
    std::cout << "\n--- Kiwipete Position ---\n";
    UltraPosition kiwi_pos;
    kiwi_pos.set_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    
    UltraMoveList kiwi_moves;
    int kiwi_count = kiwi_pos.generate_all_moves(kiwi_moves);
    std::cout << "Kiwipete position total moves: " << kiwi_count << " (should be 48)\n";
    
    int kiwi_castling = 0;
    for (int i = 0; i < kiwi_moves.size(); ++i) {
        if (kiwi_moves[i].is_castle()) {
            std::cout << "  Castling move: " << ultra_move_to_string(kiwi_moves[i]) << "\n";
            kiwi_castling++;
        }
    }
    std::cout << "Castling moves in Kiwipete: " << kiwi_castling << "\n";
    
    // Let's also check if the squares around the king are attacked
    std::cout << "\nDebugging castling conditions for Kiwipete:\n";
    // We would need access to internal methods to debug this properly
}

int main() {
    try {
        UltraAttacks::initialize();
        std::cout << "UltraEngine initialized successfully.\n";
        
        test_castling();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}