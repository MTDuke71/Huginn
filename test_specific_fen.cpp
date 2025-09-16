/**
 * @file test_specific_fen.cpp
 * @brief Test specific FEN position for debugging
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;

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

uint64_t simple_perft(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        total_nodes += simple_perft(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }

    return total_nodes;
}

void test_fen(const std::string& fen) {
    std::cout << "=== TESTING FEN: " << fen << " ===" << std::endl;
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "Failed to parse FEN!" << std::endl;
        return;
    }
    
    std::cout << "Position loaded successfully" << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "Castling rights: " << int(pos.castling_rights) << std::endl;
    std::cout << "Ply: " << pos.ply << std::endl;
    
    // Generate initial moves
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    std::cout << "Legal moves: " << moves.moves.size() << std::endl;
    
    // Test perft
    std::cout << "\nTesting perft:" << std::endl;
    for (int depth = 1; depth <= 3; depth++) {
        BitboardPosition pos_copy = pos;
        uint64_t nodes = simple_perft(pos_copy, depth);
        std::cout << "Depth " << depth << ": " << nodes << " nodes" << std::endl;
    }
}

int main() {
    std::cout << "=== SPECIFIC FEN TEST ===" << std::endl;
    
    // Test the FEN you asked about
    test_fen("r3k2r/p1ppqpb1/bn2pnp1/3PNQ2/1p2P3/2N4p/PPPBBPPP/R3K2R b KQkq - 1 1");
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    
    // For comparison, also test the original Kiwipete position
    test_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    
    return 0;
}