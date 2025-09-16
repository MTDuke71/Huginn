#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_attacks.hpp"
#include <iostream>

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

// Create f3f5 move manually 
SimpleBitboardMove create_f3f5_move() {
    SimpleBitboardMove move;
    move.from_64 = 5 + 2*8;  // f3 = f(5) + rank 3-1(2) * 8 = 21
    move.to_64 = 5 + 4*8;    // f5 = f(5) + rank 5-1(4) * 8 = 37
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
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        total_nodes += perft(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }

    return total_nodes;
}

int main() {
    std::cout << "=== F3F5 VERIFICATION TEST ===" << std::endl;
    
    // Start with Kiwipete
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cerr << "Failed to parse Kiwipete FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "Starting FEN: " << kiwipete_fen << std::endl;
    
    // Create f3f5 move
    SimpleBitboardMove f3f5_move = create_f3f5_move();
    std::cout << "Testing move: " << move_to_string(f3f5_move) << std::endl;
    
    // EXACTLY replicate what systematic analysis does:
    // 1. Make the move
    std::cout << "\nMaking f3f5 move..." << std::endl;
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(f3f5_move);
    
    // 2. Run perft depth 2 from the new position (this gives depth 3 nodes total from root)
    std::cout << "Running perft depth 2 from f3f5 position..." << std::endl;
    uint64_t nodes = perft(pos, 2);
    
    // 3. Unmake move
    pos.unmake_move(f3f5_move, undo);
    std::cout << "Move unmade" << std::endl;
    
    std::cout << "\n=== RESULTS ===" << std::endl;
    std::cout << "Nodes from systematic method: " << nodes << std::endl;
    std::cout << "Expected: 104992" << std::endl;
    std::cout << "Difference: " << (int64_t)nodes - 104992 << std::endl;
    
    if (nodes == 104992) {
        std::cout << "✓ SYSTEMATIC METHOD CORRECT" << std::endl;
    } else {
        std::cout << "✗ SYSTEMATIC METHOD HAS BUG: " << nodes << " != 104992" << std::endl;
    }
    
    return 0;
}