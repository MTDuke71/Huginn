/**
 * @file compare_move_vs_fen.cpp
 * @brief Compare f3f5 position created by move vs FEN to find state corruption
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>

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

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
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
    std::cout << "=== COMPARING MOVE VS FEN CREATION ===" << std::endl;
    
    // Method 1: Create f3f5 position by making move from this position
    std::cout << "\n--- Method 1: Create via move from provided position ---" << std::endl;
    
    BitboardPosition pos_via_move;
    pos_via_move.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    
    // Find and make f3f5 move (knight move)
    BitboardMoveList moves;
    generate_legal_moves(pos_via_move, moves);
    
    SimpleBitboardMove f3f5_move;
    bool found = false;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        if (move_str == "f3f5") {
            f3f5_move = simple_move;
            found = true;
            break;
        }
    }
    
    if (!found) {
        std::cout << "ERROR: f3f5 move not found!" << std::endl;
        std::cout << "Available moves:" << std::endl;
        for (const auto& move : moves.moves) {
            SimpleBitboardMove simple_move = convert_move(move);
            std::string move_str = move_to_string(simple_move);
            std::cout << "  " << move_str << std::endl;
        }
        return 1;
    }
    
    std::cout << "Making f3f5 move..." << std::endl;
    BitboardPosition::UndoInfo undo = pos_via_move.make_move_with_undo(f3f5_move);
    
    // Check move count
    BitboardMoveList moves_after;
    generate_legal_moves(pos_via_move, moves_after);
    std::cout << "Legal moves after f3f5: " << moves_after.moves.size() << std::endl;
    
    // Run perft
    uint64_t perft_via_move = perft(pos_via_move, 2);
    std::cout << "Perft depth 2 via move: " << perft_via_move << std::endl;
    
    // Method 2: Create f3f5 position directly from FEN
    std::cout << "\n--- Method 2: Create directly from FEN ---" << std::endl;
    
    BitboardPosition pos_via_fen;
    pos_via_fen.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PNQ2/1p2P3/2N4p/PPPBBPPP/R3K2R b KQkq - 1 1");
    
    // Check move count
    BitboardMoveList moves_direct;
    generate_legal_moves(pos_via_fen, moves_direct);
    std::cout << "Legal moves from direct FEN: " << moves_direct.moves.size() << std::endl;
    
    // Run perft
    uint64_t perft_via_fen = perft(pos_via_fen, 2);
    std::cout << "Perft depth 2 via FEN: " << perft_via_fen << std::endl;
    
    // Compare results
    std::cout << "\n--- Comparison ---" << std::endl;
    std::cout << "Move counts: via_move=" << moves_after.moves.size() 
              << " vs via_fen=" << moves_direct.moves.size() << std::endl;
    std::cout << "Perft values: via_move=" << perft_via_move 
              << " vs via_fen=" << perft_via_fen << std::endl;
    
    if (moves_after.moves.size() == moves_direct.moves.size() && perft_via_move == perft_via_fen) {
        std::cout << "✓ POSITIONS MATCH - No state corruption detected!" << std::endl;
    } else {
        std::cout << "✗ POSITIONS DIFFER - State corruption found!" << std::endl;
        std::cout << "Move difference: " << (int)moves_direct.moves.size() - (int)moves_after.moves.size() << std::endl;
        std::cout << "Perft difference: " << (int64_t)perft_via_fen - (int64_t)perft_via_move << std::endl;
    }
    
    // Cleanup
    pos_via_move.unmake_move(f3f5_move, undo);
    
    return 0;
}