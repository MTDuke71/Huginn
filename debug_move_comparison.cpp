#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_move.hpp"
#include "../ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

// Convert 64-bit square index to algebraic notation (e.g., 0 -> "a1", 7 -> "h1", 56 -> "a8")
std::string square_to_algebraic(int square) {
    int file = square % 8;  // 0-7 for a-h
    int rank = square / 8;  // 0-7 for 1-8
    char file_char = 'a' + file;
    char rank_char = '1' + rank;
    return std::string(1, file_char) + std::string(1, rank_char);
}

// Convert move to algebraic notation (e.g., e2e4)
std::string move_to_algebraic(const UltraMove& move) {
    return square_to_algebraic(move.from()) + square_to_algebraic(move.to());
}

int main() {
    UltraAttacks::initialize();
    
    // Kiwipete position FEN (CORRECT)
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    UltraPosition pos;
    if (!pos.set_fen(fen)) {
        std::cout << "Failed to parse FEN: " << fen << std::endl;
        return 1;
    }
    
    std::cout << "=== KIWIPETE POSITION ANALYSIS ===" << std::endl;
    std::cout << "FEN: " << fen << std::endl;
    std::cout << std::endl;
    
    // Reference moves (complete list of 48)
    std::set<std::string> reference_moves = {
        "a1b1", "a1c1", "a1d1", "a2a3", "a2a4", "b2b3", "c3a4", "c3b1", "c3b5", "c3d1",
        "d2c1", "d2e3", "d2f4", "d2g5", "d2h6", "d5d6", "d5e6", "e1c1", "e1d1", "e1f1", 
        "e1g1", "e2a6", "e2b5", "e2c4", "e2d1", "e2d3", "e2f1", "e5c4", "e5c6", "e5d3",
        "e5d7", "e5f7", "e5g4", "e5g6", "f3d3", "f3e3", "f3f4", "f3f5", "f3f6", "f3g3",
        "f3g4", "f3h3", "f3h5", "g2g3", "g2g4", "g2h3", "h1f1", "h1g1"
    };
    
    // Generate UltraEngine moves
    UltraMoveList move_list;
    int move_count = pos.generate_all_moves(move_list);
    
    std::set<std::string> ultra_moves;
    std::vector<std::string> ultra_moves_vec;
    
    for (int i = 0; i < move_list.size(); i++) {
        const UltraMove& move = move_list[i];
        
        if (pos.is_legal_move(move)) {
            // Convert move to algebraic notation
            std::string move_str = move_to_algebraic(move);
            ultra_moves.insert(move_str);
            ultra_moves_vec.push_back(move_str);
            
            // Debug first few moves
            if (i < 5) {
                std::cout << "DEBUG Move " << i << ": from=" << move.from() 
                          << " to=" << move.to() << " -> " << move_str << std::endl;
            }
        }
    }
    
    std::cout << "Reference moves count: " << reference_moves.size() << std::endl;
    std::cout << "UltraEngine moves count: " << ultra_moves.size() << std::endl;
    std::cout << std::endl;
    
    // Find missing moves (in reference but not in ultra)
    std::vector<std::string> missing_moves;
    std::set_difference(reference_moves.begin(), reference_moves.end(),
                       ultra_moves.begin(), ultra_moves.end(),
                       std::back_inserter(missing_moves));
    
    // Find extra moves (in ultra but not in reference)
    std::vector<std::string> extra_moves;
    std::set_difference(ultra_moves.begin(), ultra_moves.end(),
                       reference_moves.begin(), reference_moves.end(),
                       std::back_inserter(extra_moves));
    
    std::cout << "=== MISSING MOVES (in reference but not UltraEngine) ===" << std::endl;
    if (missing_moves.empty()) {
        std::cout << "None - UltraEngine has all reference moves!" << std::endl;
    } else {
        for (const auto& move : missing_moves) {
            std::cout << "MISSING: " << move << std::endl;
        }
    }
    std::cout << std::endl;
    
    std::cout << "=== EXTRA MOVES (in UltraEngine but not reference) ===" << std::endl;
    if (extra_moves.empty()) {
        std::cout << "None - UltraEngine has no extra moves!" << std::endl;
    } else {
        for (const auto& move : extra_moves) {
            std::cout << "EXTRA: " << move << std::endl;
        }
    }
    std::cout << std::endl;
    
    std::cout << "=== ALL ULTRAENGINE MOVES (sorted) ===" << std::endl;
    std::sort(ultra_moves_vec.begin(), ultra_moves_vec.end());
    for (const auto& move : ultra_moves_vec) {
        std::cout << move << std::endl;
    }
    
    return 0;
}