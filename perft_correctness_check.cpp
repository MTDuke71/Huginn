#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

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

// Perft function with legal checking
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
    std::cout << "=== Perft Correctness Analysis ===\n\n";
    
    // Initialize position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing from starting position: " << pos.to_fen() << "\n\n";
    
    // Expected perft results 
    uint64_t expected[] = {1, 20, 400, 8902, 197281, 4865609, 119060324};
    
    std::cout << std::left << std::setw(8) << "Depth" 
              << std::setw(15) << "Our Result" 
              << std::setw(15) << "Expected"
              << std::setw(12) << "Difference"
              << std::setw(10) << "Status" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    bool all_correct = true;
    int first_error_depth = -1;
    
    // Test depths 1-6 
    for (int depth = 1; depth <= 6; depth++) {
        // Reset position for each test
        pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        
        uint64_t our_result = perft_legal(pos, depth);
        uint64_t expected_result = expected[depth];
        int64_t difference = (int64_t)our_result - (int64_t)expected_result;
        
        bool correct = (our_result == expected_result);
        if (!correct && first_error_depth == -1) {
            first_error_depth = depth;
            all_correct = false;
        }
        
        std::string status = correct ? "✓ CORRECT" : "✗ WRONG";
        
        std::cout << std::left << std::setw(8) << depth
                  << std::setw(15) << our_result
                  << std::setw(15) << expected_result
                  << std::setw(12) << difference
                  << std::setw(10) << status << std::endl;
        
        // Stop if we get a wrong result to avoid long computation
        if (!correct) {
            std::cout << "\n⚠️  First error detected at depth " << depth << "\n";
            break;
        }
    }
    
    std::cout << "\n=== Analysis ===\n";
    if (all_correct) {
        std::cout << "✅ All perft results are CORRECT!\n";
        std::cout << "The engine move generation is working perfectly.\n";
    } else {
        std::cout << "❌ Error detected at depth " << first_error_depth << "\n";
        std::cout << "This suggests a bug in move generation, legal move checking,\n";
        std::cout << "or make/unmake move functionality that affects deeper searches.\n\n";
        
        if (first_error_depth <= 4) {
            std::cout << "💡 Suggested next steps:\n";
            std::cout << "1. Run perft divide at depth " << (first_error_depth-1) << " to see which moves give wrong counts\n";
            std::cout << "2. Compare our divide results with reference engine\n";
            std::cout << "3. Focus on moves that show discrepancies\n";
        }
    }
    
    return 0;
}