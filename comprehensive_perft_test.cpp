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

// Perft function 
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            nodes += perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
        }
    }
    
    return nodes;
}

int main() {
    std::cout << "=== Comprehensive Perft Test (Depths 1-6) ===\n\n";
    std::cout << "Testing from starting position: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\n\n";
    
    // Known correct perft values for starting position
    uint64_t expected_results[] = {
        0,      // depth 0 (not used)
        20,     // depth 1
        400,    // depth 2  
        8902,   // depth 3
        197281, // depth 4
        4865609,// depth 5
        119060324 // depth 6
    };
    
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Depth   Our Result      Expected        Difference   Status      Time\n";
    std::cout << "------------------------------------------------------------------------\n";
    
    bool all_correct = true;
    
    for (int depth = 1; depth <= 6; depth++) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        uint64_t our_result = perft_legal(pos, depth);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        uint64_t expected = expected_results[depth];
        int64_t difference = (int64_t)our_result - (int64_t)expected;
        
        std::cout << std::setw(2) << depth << "      " 
                  << std::setw(11) << our_result << "     " 
                  << std::setw(11) << expected << "     "
                  << std::setw(11) << difference << "    ";
        
        if (difference == 0) {
            std::cout << "✅ CORRECT";
        } else {
            std::cout << "❌ WRONG ";
            all_correct = false;
        }
        
        std::cout << "   " << std::setw(6) << duration.count() << "ms\n";
        
        // Stop testing if we find an error to avoid long waits
        if (difference != 0) {
            std::cout << "\n🛑 Error detected at depth " << depth << " - stopping tests\n";
            break;
        }
    }
    
    std::cout << "\n=== Summary ===\n";
    if (all_correct) {
        std::cout << "🎉 PERFECT: All perft results are correct through depth 6!\n";
        std::cout << "✅ En passant generation: Working perfectly\n";
        std::cout << "✅ Board wrap prevention: Fixed\n";
        std::cout << "✅ Move generation: Accurate\n";
        std::cout << "\nThe engine now has complete perft accuracy! 🚀\n";
    } else {
        std::cout << "❌ Issues remain at higher depths\n";
        std::cout << "💡 Suggestion: Use perft divide to identify specific problems\n";
    }
    
    return 0;
}