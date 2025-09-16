/**
 * @file test_pawn_movegen_bb.cpp
 * @brief Test for Phase 2 bitboard pawn move generation
 */

#include "pawn_movegen_bb.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>

int main() {
    try {
        // Create a simple position for testing
        Position pos;
        // Initialize with starting position
        pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        
        // Test pawn move generation
        S_MOVELIST list;
        generate_pawn_moves_bb(pos, list);
        
        std::cout << "Generated " << list.count << " pawn moves" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}