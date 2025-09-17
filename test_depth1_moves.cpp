#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include "ultra_engine/ultra_position.hpp"
#include "ultra_engine/ultra_move.hpp"
#include "ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

std::string square_to_algebraic(int square) {
    if (square < 0 || square > 63) return "invalid";
    
    const int file = square % 8;
    const int rank = square / 8;
    
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

void test_position(const std::string& fen, const std::string& name, int expected_depth1) {
    std::cout << "\n=== " << name << " ===\n";
    std::cout << "FEN: " << fen << "\n";
    
    UltraPosition pos;
    if (!pos.set_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN\n";
        return;
    }
    
    UltraMoveList moves;
    int count = pos.generate_all_moves(moves);
    
    std::cout << "Generated " << count << " moves (expected " << expected_depth1 << ")\n";
    
    // List all moves
    std::vector<std::string> move_list;
    for (int i = 0; i < count; i++) {
        const UltraMove& move = moves[i];
        std::string move_str = square_to_algebraic(move.from) + square_to_algebraic(move.to);
        move_list.push_back(move_str);
    }
    
    std::sort(move_list.begin(), move_list.end());
    
    std::cout << "All moves:\n";
    for (size_t i = 0; i < move_list.size(); i++) {
        std::cout << "  " << (i + 1) << ": " << move_list[i] << "\n";
    }
    
    if (count == expected_depth1) {
        std::cout << "✓ CORRECT move count!\n";
    } else {
        std::cout << "✗ WRONG move count! Difference: " << (count - expected_depth1) << "\n";
    }
}

int main() {
    std::cout << "=== DEPTH 1 MOVE GENERATION TEST ===\n";
    
    if (!UltraAttacks::init()) {
        std::cout << "ERROR: Failed to initialize attack tables\n";
        return 1;
    }
    
    // Test standard starting position
    test_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "Starting Position", 20);
    
    // Test Kiwipete
    test_position("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", "Kiwipete", 48);
    
    return 0;
}