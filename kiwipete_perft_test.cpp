#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace BitboardMoveGen;

// Convert function
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

// Perft function using the same validation as working programs
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        // All moves are already legal, so directly make and recurse
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        nodes += perft_legal(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }
    
    return nodes;
}

int main() {
    std::cout << "=== Kiwipete Position Perft Test ===\n\n";
    
    // Kiwipete position FEN
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Testing position: " << fen << "\n\n";
    
    // Known correct perft values for Kiwipete position
    uint64_t expected_results[] = {
        0,          // depth 0 (not used)
        48,         // depth 1
        2039,       // depth 2  
        97862,      // depth 3
        4085603,    // depth 4
        193690690   // depth 5
    };
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }
    
    std::cout << "Depth   Our Result      Expected        Difference   Status      Time\n";
    std::cout << "------------------------------------------------------------------------\n";
    
    for (int depth = 1; depth <= 5; depth++) {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t result = perft_legal(pos, depth);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        uint64_t expected = expected_results[depth];
        int64_t difference = static_cast<int64_t>(result) - static_cast<int64_t>(expected);
        
        std::cout << std::setw(2) << depth 
                  << std::setw(16) << result 
                  << std::setw(16) << expected 
                  << std::setw(16) << difference;
        
        if (difference == 0) {
            std::cout << "    ✓ CORRECT";
        } else {
            std::cout << "    ✗ ERROR  ";
        }
        
        std::cout << std::setw(8) << duration.count() << "ms\n";
    }
    
    std::cout << "\n=== Summary ===\n";
    bool all_correct = true;
    for (int depth = 1; depth <= 5; depth++) {
        uint64_t result = perft_legal(pos, depth);
        if (result != expected_results[depth]) {
            all_correct = false;
            break;
        }
    }
    
    if (all_correct) {
        std::cout << "🎯 PERFECT: All Kiwipete perft results are correct through depth 5!\n";
        std::cout << "✓ Complex position handling: Working perfectly\n";
        std::cout << "✓ Castling generation: Accurate\n";  
        std::cout << "✓ Move generation: Complete\n";
        std::cout << "\nThe engine handles complex positions correctly! 🏆\n";
    } else {
        std::cout << "❌ Some results are incorrect. Move generation needs debugging.\n";
    }
    
    return 0;
}