/**
 * @file debug_ultra_position.cpp
 * @brief Debug UltraPosition functionality step by step
 */
#include <iostream>
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_move.hpp"
#include "../ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

void test_starting_position() {
    std::cout << "=== Testing UltraPosition Starting Position ===\n";
    
    UltraPosition pos;
    std::string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    std::cout << "Setting FEN: " << starting_fen << "\n";
    bool success = pos.set_fen(starting_fen);
    
    if (!success) {
        std::cout << "ERROR: Failed to set starting FEN!\n";
        return;
    }
    
    std::cout << "FEN parsing successful.\n";
    
    // Test move generation
    UltraMoveList moves;
    int move_count = pos.generate_all_moves(moves);
    
    std::cout << "Generated " << move_count << " moves (should be 20)\n";
    std::cout << "Move list size: " << moves.size() << "\n";
    
    // Print first few moves for inspection
    std::cout << "First 10 moves:\n";
    for (int i = 0; i < std::min(10, moves.size()); ++i) {
        const UltraMove& move = moves[i];
        std::cout << "  Move " << i << ": from=" << move.from() 
                  << " to=" << move.to() 
                  << " capture=" << move.captured()
                  << " promotion=" << move.promoted() 
                  << " special=" << (move.is_special() ? "yes" : "no") << "\n";
    }
    
    // Test legality of each move
    int legal_count = 0;
    for (int i = 0; i < moves.size(); ++i) {
        if (pos.is_legal_move(moves[i])) {
            legal_count++;
        }
    }
    
    std::cout << "Legal moves: " << legal_count << " out of " << moves.size() << "\n";
}

void test_kiwipete_position() {
    std::cout << "\n=== Testing UltraPosition Kiwipete Position ===\n";
    
    UltraPosition pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    std::cout << "Setting FEN: " << kiwipete_fen << "\n";
    bool success = pos.set_fen(kiwipete_fen);
    
    if (!success) {
        std::cout << "ERROR: Failed to set Kiwipete FEN!\n";
        return;
    }
    
    std::cout << "FEN parsing successful.\n";
    
    // Test move generation
    UltraMoveList moves;
    int move_count = pos.generate_all_moves(moves);
    
    std::cout << "Generated " << move_count << " moves (should be 48)\n";
    std::cout << "Move list size: " << moves.size() << "\n";
    
    // Test legality
    int legal_count = 0;
    for (int i = 0; i < moves.size(); ++i) {
        if (pos.is_legal_move(moves[i])) {
            legal_count++;
        }
    }
    
    std::cout << "Legal moves: " << legal_count << " out of " << moves.size() << "\n";
}

int main() {
    try {
        // Initialize UltraEngine
        UltraAttacks::initialize();
        std::cout << "UltraEngine initialized successfully.\n\n";
        
        test_starting_position();
        test_kiwipete_position();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}