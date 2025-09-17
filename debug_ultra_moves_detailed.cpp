/**
 * @file debug_ultra_moves_detailed.cpp
 * @brief Detailed debugging of UltraEngine move generation by piece type
 */
#include <iostream>
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_move.hpp"
#include "../ultra_engine/ultra_attacks.hpp"
#include "../ultra_engine/ultra_knight_gen.hpp"
#include "../ultra_engine/ultra_sliding_gen.hpp"
#include "../ultra_engine/ultra_pawn_gen.hpp"

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

void debug_move_generation_by_piece() {
    std::cout << "=== Detailed UltraEngine Move Generation Debug ===\n";
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    UltraPosition pos;
    pos.set_fen(kiwipete_fen);
    
    int color = pos.get_to_move();
    UltraMoveList moves;
    int total_count = 0;
    
    // Test each piece type separately
    std::cout << "\n--- Pawn Moves ---\n";
    UltraMoveList pawn_moves;
    int pawn_count = UltraPawnGen<UltraPosition>::generate_all(pos, pawn_moves, color);
    std::cout << "Pawn moves: " << pawn_count << "\n";
    for (int i = 0; i < pawn_moves.size(); ++i) {
        std::cout << "  " << ultra_move_to_string(pawn_moves[i]) << "\n";
    }
    total_count += pawn_count;
    
    std::cout << "\n--- Knight Moves ---\n";
    UltraMoveList knight_moves;
    int knight_count = UltraKnightGen<UltraPosition>::generate_all(pos, knight_moves, color);
    std::cout << "Knight moves: " << knight_count << "\n";
    for (int i = 0; i < knight_moves.size(); ++i) {
        std::cout << "  " << ultra_move_to_string(knight_moves[i]) << "\n";
    }
    total_count += knight_count;
    
    std::cout << "\n--- Sliding Piece Moves (Bishop, Rook, Queen) ---\n";
    UltraMoveList sliding_moves;
    int sliding_count = UltraSlidingGen<UltraPosition>::generate_all(pos, sliding_moves, color);
    std::cout << "Sliding moves: " << sliding_count << "\n";
    for (int i = 0; i < sliding_moves.size(); ++i) {
        std::cout << "  " << ultra_move_to_string(sliding_moves[i]) << "\n";
    }
    total_count += sliding_count;
    
    std::cout << "\n--- Combined Generation Test ---\n";
    
    // Now test the combined generation
    UltraMoveList all_moves;
    int all_count = pos.generate_all_moves(all_moves);
    std::cout << "Combined generation: " << all_count << "\n";
    std::cout << "Expected: 48\n";
    
    // Count by type in the combined generation
    int pawn_in_all = 0, knight_in_all = 0, sliding_in_all = 0, king_in_all = 0, castle_in_all = 0;
    
    for (int i = 0; i < all_moves.size(); ++i) {
        // This is a crude classification - we'd need position info to be accurate
        if (all_moves[i].is_castle()) {
            castle_in_all++;
        }
        // Add more classification logic here if needed
    }
    
    std::cout << "\n--- Summary ---\n";
    std::cout << "Pawn moves: " << pawn_count << "\n";
    std::cout << "Knight moves: " << knight_count << "\n";
    std::cout << "Sliding moves: " << sliding_count << "\n";
    std::cout << "Total by pieces: " << total_count << "\n";
    std::cout << "Combined generation: " << all_count << "\n";
    std::cout << "Missing: " << (48 - all_count) << " moves\n";
}

int main() {
    try {
        UltraAttacks::initialize();
        std::cout << "UltraEngine initialized successfully.\n";
        
        debug_move_generation_by_piece();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}