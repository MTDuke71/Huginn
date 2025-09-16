#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;
using namespace std;

// Convert function
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

// Perft function 
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            nodes += perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
        }
    }
    
    return nodes;
}

int main() {
    cout << "=== Perft Depth 1-3 Test: After a2a3 ===" << endl;
    
    // Start from position after a2a3
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    cout << "Position: rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1" << endl << endl;
    
    // Test depths 1-3 to isolate the problem
    for (int depth = 1; depth <= 3; depth++) {
        BitboardPosition test_pos = pos;  // Copy for testing
        uint64_t result = perft_legal(test_pos, depth);
        cout << "Depth " << depth << ": " << result << " nodes" << endl;
    }
    
    cout << endl << "Expected values:" << endl;
    cout << "Depth 1: 20 (confirmed by move generation test)" << endl;
    cout << "Depth 2: ~400 (typical for this position type)" << endl;
    cout << "Depth 3: ~8000-9000 (estimate)" << endl;
    
    return 0;
}