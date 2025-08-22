#include "position.hpp"
#include "board.hpp"
#include "debug.hpp"
#include "init.hpp"
#include "move.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>
#include <vector>

using namespace std;
using namespace Huginn;

uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    MoveList list; 
    generate_legal_moves(pos, list);
    uint64_t nodes = 0;
    for (const auto& m : list.v) {
        pos.make_move_with_undo(m);
        nodes += perft(pos, depth - 1);
        pos.undo_move();
    }
    return nodes;
}

int main() {
    init();
    
    cout << "=== Debugging Kiwipete Position ===\n";
    Position pos;
    string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    pos.set_from_fen(fen);
    
    cout << "FEN: " << fen << "\n";
    
    // Test depth 1 first
    cout << "\n=== Depth 1 Test ===\n";
    MoveList moves;
    generate_legal_moves(pos, moves);
    cout << "Generated " << moves.size() << " moves\n";
    
    uint64_t depth1_result = perft(pos, 1);
    cout << "Depth 1 result: " << depth1_result << "\n";
    cout << "Expected depth 1: 48\n";
    
    if (depth1_result == 48) {
        cout << "Depth 1 PASSED!\n";
        
        // Now test depth 2
        cout << "\n=== Depth 2 Test ===\n";
        uint64_t depth2_result = perft(pos, 2);
        cout << "Depth 2 result: " << depth2_result << "\n";
        cout << "Expected depth 2: 2039\n";
        
        if (depth2_result == 2039) {
            cout << "Depth 2 PASSED!\n";
            
            // Now test depth 3
            cout << "\n=== Depth 3 Test ===\n";
            uint64_t depth3_result = perft(pos, 3);
            cout << "Depth 3 result: " << depth3_result << "\n";
            cout << "Expected depth 3: 97862\n";
            cout << "Difference: " << (int64_t)depth3_result - 97862 << "\n";
        } else {
            cout << "Depth 2 FAILED! Difference: " << (int64_t)depth2_result - 2039 << "\n";
        }
    } else {
        cout << "Depth 1 FAILED! Difference: " << (int64_t)depth1_result - 48 << "\n";
    }
    
    return 0;
}
