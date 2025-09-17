#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
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

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

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
    std::cout << "=== Debugging Knight Capture Issues ===\n";
    
    // Kiwipete position
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    BitboardPosition pos;
    pos.set_from_fen(fen);
    
    std::cout << "Original position: " << fen << "\n\n";
    
    // Test the specific knight capture e5f7
    SimpleBitboardMove e5f7;
    e5f7.from_64 = 36; // e5
    e5f7.to_64 = 53;   // f7
    e5f7.is_capture = true;
    e5f7.is_ep_capture = false;
    e5f7.is_castling = false;
    e5f7.is_promotion = false;
    e5f7.promotion_type = static_cast<PieceType>(0);
    
    std::cout << "Testing knight capture e5f7...\n";
    
    // Check what piece is being captured
    Piece captured = pos.piece_at(53);
    std::cout << "Piece at f7 before capture: " << static_cast<int>(captured) << "\n";
    
    // Make the move and check what was captured
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(e5f7);
    std::cout << "Captured piece in undo info: " << static_cast<int>(undo.captured_piece) << "\n";
    std::cout << "Moving piece type in undo info: " << static_cast<int>(undo.moving_piece_type) << "\n";
    
    // Check position after move
    std::cout << "Position after e5f7: ";
    std::cout << pos.to_fen() << "\n";
    
    // Test perft from this position
    uint64_t nodes_d3 = perft(pos, 3);
    std::cout << "Perft depth 3 after e5f7: " << nodes_d3 << " nodes\n";
    
    // Unmake and verify
    pos.unmake_move(e5f7, undo);
    std::cout << "Position after unmake: ";
    std::cout << pos.to_fen() << "\n";
    
    // Check if position is restored correctly
    if (pos.to_fen() == fen) {
        std::cout << "✓ Position correctly restored\n";
    } else {
        std::cout << "✗ Position NOT correctly restored!\n";
    }
    
    // Now test similar for e5g6
    std::cout << "\n=== Testing e5g6 ===\n";
    SimpleBitboardMove e5g6;
    e5g6.from_64 = 36; // e5
    e5g6.to_64 = 46;   // g6
    e5g6.is_capture = true;
    e5g6.is_ep_capture = false;
    e5g6.is_castling = false;
    e5g6.is_promotion = false;
    e5g6.promotion_type = static_cast<PieceType>(0);
    
    // Check what piece is being captured
    captured = pos.piece_at(46);
    std::cout << "Piece at g6 before capture: " << static_cast<int>(captured) << "\n";
    
    undo = pos.make_move_with_undo(e5g6);
    std::cout << "Captured piece in undo info: " << static_cast<int>(undo.captured_piece) << "\n";
    
    // Check position after move
    std::cout << "Position after e5g6: ";
    std::cout << pos.to_fen() << "\n";
    
    // Test perft from this position
    nodes_d3 = perft(pos, 3);
    std::cout << "Perft depth 3 after e5g6: " << nodes_d3 << " nodes\n";
    
    // Unmake and verify
    pos.unmake_move(e5g6, undo);
    if (pos.to_fen() == fen) {
        std::cout << "✓ Position correctly restored\n";
    } else {
        std::cout << "✗ Position NOT correctly restored!\n";
    }
    
    return 0;
}