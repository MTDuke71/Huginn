/**
 * @file compare_move_generation.cpp
 * @brief Compare legacy vs UltraEngine move generation
 */
#include <iostream>
#include <set>
#include <string>
#include "../src/position.hpp"
#include "../src/bitboard_movegen.hpp"
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_move.hpp"
#include "../ultra_engine/ultra_attacks.hpp"
#include "../src/init.hpp"

using namespace UltraEngine;

// Convert 64-bit square to algebraic notation
std::string square_to_algebraic(int sq64) {
    char file = 'a' + (sq64 % 8);
    char rank = '1' + (sq64 / 8);
    return std::string(1, file) + std::string(1, rank);
}

// Convert UltraMove to string
std::string ultra_move_to_string(const UltraMove& move) {
    std::string result = square_to_algebraic(move.from()) + square_to_algebraic(move.to());
    if (move.is_promotion()) {
        char pieces[] = {' ', 'n', 'b', 'r', 'q', ' ', 'k'};
        if (move.promoted() >= 1 && move.promoted() <= 5) {
            result += pieces[move.promoted()];
        }
    }
    return result;
}

// Convert S_MOVE to string for comparison
std::string legacy_move_to_string(const S_MOVE& move) {
    // This is a simplified conversion - we'd need the actual conversion logic
    // For now, just return a placeholder
    return "legacy_move";
}

void compare_kiwipete_moves() {
    std::cout << "=== Comparing Move Generation for Kiwipete Position ===\n";
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    // Test UltraEngine
    std::cout << "\n--- UltraEngine Move Generation ---\n";
    UltraPosition ultra_pos;
    ultra_pos.set_fen(kiwipete_fen);
    
    UltraMoveList ultra_moves;
    int ultra_count = ultra_pos.generate_all_moves(ultra_moves);
    
    std::cout << "UltraEngine generated " << ultra_count << " moves:\n";
    std::set<std::string> ultra_move_set;
    
    for (int i = 0; i < ultra_moves.size(); ++i) {
        std::string move_str = ultra_move_to_string(ultra_moves[i]);
        ultra_move_set.insert(move_str);
        std::cout << "  " << i+1 << ": " << move_str;
        if (ultra_moves[i].is_capture()) std::cout << " (capture)";
        if (ultra_moves[i].is_promotion()) std::cout << " (promotion)";
        if (ultra_moves[i].is_castle()) std::cout << " (castle)";
        if (ultra_moves[i].is_ep()) std::cout << " (en passant)";
        std::cout << "\n";
    }
    
    // Test Legacy System
    std::cout << "\n--- Legacy Move Generation ---\n";
    Huginn::init(); // Initialize legacy system
    
    Position legacy_pos;
    legacy_pos.SetBoard(kiwipete_fen);
    
    S_MOVELIST legacy_moves;
    legacy_pos.generate_all_moves(legacy_moves);
    
    std::cout << "Legacy system generated " << legacy_moves.count << " moves\n";
    
    // The real comparison would require converting both move formats to a common representation
    // For now, let's focus on the count difference
    
    std::cout << "\n--- Summary ---\n";
    std::cout << "UltraEngine: " << ultra_count << " moves\n";
    std::cout << "Legacy:      " << legacy_moves.count << " moves\n";
    std::cout << "Difference:  " << (legacy_moves.count - ultra_count) << " moves\n";
}

int main() {
    try {
        // Initialize both systems
        UltraAttacks::initialize();
        std::cout << "UltraEngine initialized successfully.\n";
        
        compare_kiwipete_moves();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}