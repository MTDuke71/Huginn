/**
 * @file test_bitboard_movegen.cpp
 * @brief Simple test to verify bitboard move generation works
 */

#include "position.hpp"
#include "bitboard_movegen.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>

int main() {
    std::cout << "=== Testing Bitboard Move Generation ===\n";
    
    try {
        // Create starting position
        Position pos;
        if (!pos.from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
            std::cout << "Failed to set starting position\n";
            return 1;
        }
        
        std::cout << "Position loaded: starting position\n";
        
        // Test bitboard move generation
        S_MOVELIST bitboard_moves;
        BitboardMoveGen::generate_all_moves_bitboard(pos, bitboard_moves);
        
        std::cout << "Bitboard move generation: " << bitboard_moves.count << " moves\n";
        
        // Test normal move generation (should be same as bitboard in huginn2)
        S_MOVELIST normal_moves;
        generate_all_moves(pos, normal_moves);
        
        std::cout << "Normal move generation: " << normal_moves.count << " moves\n";
        
        if (bitboard_moves.count == normal_moves.count) {
            std::cout << "SUCCESS: Both methods generate same number of moves!\n";
            std::cout << "The bitboard integration is working correctly.\n";
            return 0;
        } else {
            std::cout << "ERROR: Move count mismatch!\n";
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cout << "Unknown exception\n";
        return 1;
    }
}