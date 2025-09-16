/**
 * @file simple_bitboard_debug.cpp
 * @brief Simple test to isolate BitboardPosition bugs
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

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

void test_f3f5_directly() {
    std::cout << "=== TESTING F3F5 MOVE DIRECTLY ===" << std::endl;
    
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cout << "Failed to parse FEN!" << std::endl;
        return;
    }
    
    std::cout << "Initial position loaded successfully" << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "Castling rights: " << int(pos.castling_rights) << std::endl;
    
    // Generate initial moves
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    std::cout << "Legal moves from root: " << moves.moves.size() << std::endl;
    
    // Find f3f5 move
    SimpleBitboardMove f3f5_move;
    bool found = false;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        if (move_str == "f3f5") {
            f3f5_move = simple_move;
            found = true;
            std::cout << "Found f3f5 move: " << simple_move.from_64 << " -> " << simple_move.to_64 << std::endl;
            break;
        }
    }
    
    if (!found) {
        std::cout << "f3f5 move not found!" << std::endl;
        return;
    }
    
    // Test the move
    std::cout << "\nMaking f3f5 move..." << std::endl;
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(f3f5_move);
    
    std::cout << "After f3f5:" << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "Castling rights: " << int(pos.castling_rights) << std::endl;
    std::cout << "Ply: " << pos.ply << std::endl;
    
    // Test perft from f3f5 position
    std::cout << "\nTesting perft from f3f5 position:" << std::endl;
    for (int depth = 1; depth <= 3; depth++) {
        BitboardPosition pos_copy = pos;
        uint64_t nodes = simple_perft(pos_copy, depth);
        std::cout << "Depth " << depth << ": " << nodes << " nodes" << std::endl;
        
        if (depth == 3) {
            std::cout << "Expected: 104992 nodes" << std::endl;
            if (nodes != 104992) {
                std::cout << "MISMATCH! Difference: " << int64_t(nodes) - 104992 << std::endl;
            } else {
                std::cout << "MATCH!" << std::endl;
            }
        }
    }
    
    // Undo the move
    std::cout << "\nUndoing f3f5 move..." << std::endl;
    pos.unmake_move(f3f5_move, undo);
    
    std::cout << "After undo:" << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "Castling rights: " << int(pos.castling_rights) << std::endl;
    std::cout << "Ply: " << pos.ply << std::endl;
}

int main() {
    std::cout << "=== SIMPLE BITBOARDPOSITION DEBUG ===" << std::endl;
    
    test_f3f5_directly();
    
    return 0;
}