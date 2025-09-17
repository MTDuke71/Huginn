/**
 * @file debug_fen_parsing.cpp
 * @brief Debug FEN parsing for Kiwipete position
 */
#include <iostream>
#include <bitset>
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

int main() {
    UltraAttacks::initialize();
    
    std::string fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    std::cout << "=== FEN PARSING DEBUG ===" << std::endl;
    std::cout << "Target FEN: " << fen << std::endl;
    
    UltraPosition pos;
    bool success = pos.set_fen(fen);
    
    if (!success) {
        std::cout << "ERROR: Failed to parse FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "FEN parsing successful" << std::endl;
    
    // Let's examine the board state by checking a few key squares
    std::cout << "\n=== BOARD STATE VERIFICATION ===" << std::endl;
    
    // Test a few key squares from Kiwipete position:
    // a1 should have white rook, e1 should have white king, etc.
    
    std::cout << "Testing key squares (0-63 indexing where a1=0, h1=7, a8=56, h8=63):" << std::endl;
    
    // This would require access to board state methods we might not have
    // For now, let's just generate a few moves and see their coordinates
    
    UltraMoveList moves;
    int move_count = pos.generate_all_moves(moves);
    
    std::cout << "Generated " << move_count << " total moves" << std::endl;
    std::cout << "First 10 moves (internal coordinates):" << std::endl;
    
    for (int i = 0; i < std::min(10, moves.size()); ++i) {
        const UltraMove& move = moves[i];
        if (pos.is_legal_move(move)) {
            std::cout << "  Move " << i << ": from=" << move.from() 
                      << " to=" << move.to() << " (binary: " 
                      << std::bitset<6>(move.from()) << " -> " 
                      << std::bitset<6>(move.to()) << ")" << std::endl;
        }
    }
    
    return 0;
}