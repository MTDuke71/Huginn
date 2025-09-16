/**
 * @file verify_kiwipete_perft.cpp
 * @brief Verify Kiwipete perft against known correct values
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace BitboardMoveGen;

// Helper function to convert move for compatibility
SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.is_capture = move.is_capture;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    simple_move.is_promotion = move.is_promotion;
    simple_move.promotion_type = move.promotion_type;
    return simple_move;
}

// Simple perft function
uint64_t perft(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        total_nodes += perft(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }

    return total_nodes;
}

int main() {
    std::cout << "=== VERIFY KIWIPETE PERFT VALUES ===" << std::endl;
    
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "FEN: " << fen << std::endl;
    
    // Known correct values for Kiwipete position
    std::vector<uint64_t> expected = {1, 48, 2039, 97862, 4085603, 193690690};
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cerr << "Failed to parse FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "\nPerft verification:" << std::endl;
    std::cout << "Depth | Expected   | Actual     | Difference | Status" << std::endl;
    std::cout << "------|------------|------------|------------|--------" << std::endl;
    
    bool all_correct = true;
    
    for (int depth = 1; depth <= 5; depth++) {
        BitboardPosition pos_copy = pos;  // Make a copy for each test
        uint64_t actual = perft(pos_copy, depth);
        uint64_t expect = expected[depth];
        int64_t diff = (int64_t)actual - (int64_t)expect;
        
        std::string status = (actual == expect) ? "✓ PASS" : "✗ FAIL";
        if (actual != expect) all_correct = false;
        
        std::cout << std::setw(5) << depth 
                  << " | " << std::setw(10) << expect
                  << " | " << std::setw(10) << actual
                  << " | " << std::setw(10) << diff
                  << " | " << status << std::endl;
    }
    
    std::cout << "\n=== SUMMARY ===" << std::endl;
    if (all_correct) {
        std::cout << "🎉 ALL PERFT VALUES CORRECT!" << std::endl;
        std::cout << "BitboardPosition is working perfectly." << std::endl;
    } else {
        std::cout << "❌ PERFT MISMATCHES DETECTED!" << std::endl;
        std::cout << "BitboardPosition has bugs that need investigation." << std::endl;
        
        // Check if it's systematic undercounting
        bool all_under = true;
        for (int depth = 1; depth <= 5; depth++) {
            BitboardPosition pos_copy = pos;
            uint64_t actual = perft(pos_copy, depth);
            if (actual > expected[depth]) {
                all_under = false;
                break;
            }
        }
        
        if (all_under) {
            std::cout << "\nPattern: Systematic undercounting detected." << std::endl;
            std::cout << "This suggests moves are being missed or incorrectly handled." << std::endl;
        }
    }
    
    return 0;
}