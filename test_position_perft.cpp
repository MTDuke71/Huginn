/**
 * @file test_position_perft.cpp
 * @brief Test perft 1-5 from specific position
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
    std::cout << "=== PERFT TEST FOR SPECIFIC POSITION ===" << std::endl;
    
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "FEN: " << fen << std::endl;
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cerr << "Failed to parse FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "\nPerft results:" << std::endl;
    std::cout << "Depth | Nodes" << std::endl;
    std::cout << "------|----------" << std::endl;
    
    for (int depth = 1; depth <= 5; depth++) {
        BitboardPosition pos_copy = pos;  // Make a copy for each test
        uint64_t nodes = perft(pos_copy, depth);
        std::cout << std::setw(5) << depth << " | " << std::setw(8) << nodes << std::endl;
    }
    
    return 0;
}