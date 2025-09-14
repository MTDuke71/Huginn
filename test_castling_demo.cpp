/**
 * @file test_castling_demo.cpp
 * @brief Quick test to verify castling moves are generated with king lookup tables
 */

#include "king_lookup_tables.hpp"
#include "position.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include "board120.hpp"
#include <iostream>

std::string sq120_to_algebraic(int sq120) {
    if (sq120 < 21 || sq120 > 98) return "invalid";
    int file = (sq120 - 21) % 10;
    int rank = (sq120 - 21) / 10;
    if (file > 7 || rank > 7) return "invalid";
    
    std::string result;
    result += ('a' + file);
    result += ('1' + rank);
    return result;
}

int main() {
    // Initialize king lookup tables
    KingLookupTables::initialize_king_tables();
    
    std::cout << "=== Castling Move Generation Test ===\n";
    
    // Test position where castling is possible
    // Remove pieces between king and rook
    Position pos;
    pos.set_from_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
    
    std::cout << "Position: " << pos.to_fen() << "\n";
    std::cout << "King position: " << sq120_to_algebraic(pos.king_sq[0]) << "\n";
    std::cout << "Castling rights: " << (int)pos.castling_rights << "\n";
    
    // Generate moves using king lookup tables
    S_MOVELIST list;
    KingLookupTables::generate_king_moves_lookup(pos, list, Color::White);
    
    std::cout << "White king moves generated: " << list.count << "\n";
    for (int i = 0; i < list.count; ++i) {
        const auto& move = list.moves[i].move;
        int from = (move & MOVE_FROM_MASK) >> MOVE_FROM_SHIFT;
        int to = (move & MOVE_TO_MASK) >> MOVE_TO_SHIFT;
        bool is_castle = (move & MOVE_CASTLE) != 0;
        
        std::cout << "  Move " << (i+1) << ": " 
                  << sq120_to_algebraic(from) << "-" << sq120_to_algebraic(to);
        if (is_castle) std::cout << " (CASTLE)";
        std::cout << "\n";
    }
    
    return 0;
}