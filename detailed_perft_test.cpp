#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <chrono>
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

// Perft divide function to show breakdown
uint64_t perft_divide(BitboardPosition& pos, int depth) {
    if (depth <= 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t total_nodes = 0;
    
    std::cout << "Divide " << depth << " breakdown:\n";
    std::cout << "========================\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t nodes = perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
            
            // Convert move to algebraic notation
            char from_file = 'a' + (simple_move.from_64 % 8);
            char from_rank = '1' + (simple_move.from_64 / 8);
            char to_file = 'a' + (simple_move.to_64 % 8);
            char to_rank = '1' + (simple_move.to_64 / 8);
            
            std::cout << "  " << from_file << from_rank << to_file << to_rank 
                      << ": " << std::setw(8) << nodes << "\n";
            
            total_nodes += nodes;
        }
    }
    
    std::cout << "------------------------\n";
    std::cout << "Total: " << total_nodes << "\n\n";
    
    return total_nodes;
}

int main() {
    std::cout << "=== Detailed Perft Analysis ===\n\n";
    
    // Initialize position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing from starting position: " << pos.to_fen() << "\n\n";
    
    // Test depth 4 with breakdown
    std::cout << "=== DEPTH 4 ANALYSIS ===\n";
    auto start = std::chrono::high_resolution_clock::now();
    uint64_t result4 = perft_divide(pos, 4);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Depth 4 result: " << result4 << " (expected: 197281)\n";
    std::cout << "Time: " << duration.count() << " ms\n";
    std::cout << "Status: " << (result4 == 197281 ? "✓ CORRECT" : "✗ WRONG") << "\n\n";
    
    if (result4 != 197281) {
        std::cout << "⚠️  STOPPING - Depth 4 is incorrect, won't test depth 5\n";
        return 1;
    }
    
    return 0;
}