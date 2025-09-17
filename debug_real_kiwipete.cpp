/**
 * @file debug_real_kiwipete.cpp
 * @brief Debug the real standard Kiwipete position
 */

#include <iostream>
#include <vector>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

int perft_original(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    MoveList moves;
    generate_moves_bitboard_pure(pos, moves);
    
    int nodes = 0;
    for (const auto& move : moves.moves) {
        pos.make_move(move);
        nodes += perft_original(pos, depth - 1);
        pos.unmake_move();
    }
    return nodes;
}

int perft_optimized(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_moves_optimized(pos, moves);
    
    int nodes = 0;
    for (const auto& move : moves.moves) {
        pos.make_move_simple(move);
        nodes += perft_optimized(pos, depth - 1);
        pos.unmake_move();
    }
    return nodes;
}

int main() {
    cout << "=== Debugging Real Kiwipete Position ===" << endl;
    
    // Initialize
    init_attack_tables();
    
    // Real Kiwipete position
    string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    cout << "FEN: " << fen << endl;
    
    BitboardPosition pos1, pos2;
    pos1.set_from_fen(fen);
    pos2.set_from_fen(fen);
    
    cout << "\n=== Depth 1 Test ===" << endl;
    int orig1 = perft_original(pos1, 1);
    int opt1 = perft_optimized(pos2, 1);
    cout << "Original: " << orig1 << endl;
    cout << "Optimized: " << opt1 << endl;
    cout << "Difference: " << opt1 - orig1 << endl;
    
    if (orig1 == opt1) {
        cout << "\n=== Depth 2 Test ===" << endl;
        pos1.set_from_fen(fen);
        pos2.set_from_fen(fen);
        
        int orig2 = perft_original(pos1, 2);
        int opt2 = perft_optimized(pos2, 2);
        cout << "Original: " << orig2 << endl;
        cout << "Optimized: " << opt2 << endl;
        cout << "Difference: " << opt2 - orig2 << endl;
        cout << "Expected: 2039" << endl;
    }
    
    return 0;
}