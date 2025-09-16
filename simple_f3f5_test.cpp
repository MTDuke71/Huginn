#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_attacks.hpp"
#include <iostream>

using namespace BitboardMoveGen;

// Create f3f5 move manually 
SimpleBitboardMove create_f3f5_move() {
    SimpleBitboardMove move;
    move.from_64 = 21; // f3
    move.to_64 = 37;   // f5  
    move.is_capture = false;
    move.is_ep_capture = false;
    move.is_castling = false;
    move.is_promotion = false;
    move.promotion_type = static_cast<PieceType>(0);
    return move;
}

// Simple perft function
uint64_t perft(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move;
        simple_move.from_64 = move.from_64;
        simple_move.to_64 = move.to_64;
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        simple_move.promotion_type = move.promotion_type;
        
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        total_nodes += perft(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }

    return total_nodes;
}

int main() {
    std::cout << "=== F3F5 PERFT VERIFICATION ===" << std::endl;
    
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cerr << "Failed to parse Kiwipete FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "Starting from Kiwipete position" << std::endl;
    
    // Make f3f5 move
    SimpleBitboardMove f3f5_move = create_f3f5_move();
    std::cout << "Making f3f5 move..." << std::endl;
    
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(f3f5_move);
    
    // Test different perft depths from f3f5 position
    std::cout << "\nPerft results from f3f5 position:" << std::endl;
    for (int depth = 1; depth <= 3; depth++) {
        uint64_t nodes = perft(pos, depth);
        std::cout << "Depth " << depth << ": " << nodes << " nodes" << std::endl;
    }
    
    pos.unmake_move(f3f5_move, undo);
    std::cout << "\nMove unmade successfully" << std::endl;
    
    return 0;
}