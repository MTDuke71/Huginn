#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>

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
    std::cout << "=== Knight Move Perft Analysis ===\n";
    
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    BitboardPosition pos;
    pos.set_from_fen(fen);
    
    // Known reference values from compare_depth5_reference output
    uint64_t ref_e5f7 = 4164923;
    uint64_t ref_e5g6 = 3949417;
    
    std::cout << "Reference values:\n";
    std::cout << "e5f7: " << ref_e5f7 << "\n";
    std::cout << "e5g6: " << ref_e5g6 << "\n\n";
    
    // Test e5f7
    SimpleBitboardMove e5f7;
    e5f7.from_64 = 36; e5f7.to_64 = 53; e5f7.is_capture = true;
    e5f7.is_ep_capture = false; e5f7.is_castling = false; e5f7.is_promotion = false;
    
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(e5f7);
    uint64_t actual_e5f7 = perft(pos, 4);
    pos.unmake_move(e5f7, undo);
    
    std::cout << "e5f7 analysis:\n";
    std::cout << "Expected: " << ref_e5f7 << "\n";
    std::cout << "Actual:   " << actual_e5f7 << "\n";
    std::cout << "Diff:     " << (int64_t)actual_e5f7 - (int64_t)ref_e5f7 << "\n\n";
    
    // Test e5g6
    SimpleBitboardMove e5g6;
    e5g6.from_64 = 36; e5g6.to_64 = 46; e5g6.is_capture = true;
    e5g6.is_ep_capture = false; e5g6.is_castling = false; e5g6.is_promotion = false;
    
    undo = pos.make_move_with_undo(e5g6);
    uint64_t actual_e5g6 = perft(pos, 4);
    pos.unmake_move(e5g6, undo);
    
    std::cout << "e5g6 analysis:\n";
    std::cout << "Expected: " << ref_e5g6 << "\n";
    std::cout << "Actual:   " << actual_e5g6 << "\n";
    std::cout << "Diff:     " << (int64_t)actual_e5g6 - (int64_t)ref_e5g6 << "\n\n";
    
    // Test some correct moves for comparison
    uint64_t ref_g2g3 = 3472039;
    SimpleBitboardMove g2g3;
    g2g3.from_64 = 14; g2g3.to_64 = 22; g2g3.is_capture = false;
    g2g3.is_ep_capture = false; g2g3.is_castling = false; g2g3.is_promotion = false;
    
    undo = pos.make_move_with_undo(g2g3);
    uint64_t actual_g2g3 = perft(pos, 4);
    pos.unmake_move(g2g3, undo);
    
    std::cout << "g2g3 (should be correct):\n";
    std::cout << "Expected: " << ref_g2g3 << "\n";
    std::cout << "Actual:   " << actual_g2g3 << "\n";
    std::cout << "Diff:     " << (int64_t)actual_g2g3 - (int64_t)ref_g2g3 << "\n";
    
    return 0;
}