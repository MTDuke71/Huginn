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
    std::cout << "=== Testing Cold vs Warm Perft Execution ===\n";
    
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "FEN: " << fen << "\n\n";
    
    // Test 1: Run depth 4 immediately (cold start)
    std::cout << "=== Test 1: Cold Start (depth 4 directly) ===\n";
    BitboardPosition pos1;
    if (!pos1.set_from_fen(fen)) {
        std::cerr << "Failed to parse FEN!" << std::endl;
        return 1;
    }
    
    uint64_t cold_depth4 = perft(pos1, 4);
    std::cout << "Cold depth 4 result: " << cold_depth4 << "\n";
    std::cout << "Expected depth 4:    " << 4085603 << "\n";
    std::cout << "Difference:          " << (int64_t)cold_depth4 - 4085603 << "\n\n";
    
    // Test 2: Warm up with depths 1-3, then run depth 4 (warm start)
    std::cout << "=== Test 2: Warm Start (depths 1-3 first) ===\n";
    BitboardPosition pos2;
    if (!pos2.set_from_fen(fen)) {
        std::cerr << "Failed to parse FEN!" << std::endl;
        return 1;
    }
    
    // Warm up runs
    BitboardPosition pos2_copy1 = pos2;
    uint64_t warm_depth1 = perft(pos2_copy1, 1);
    std::cout << "Warm depth 1: " << warm_depth1 << " (expected: 48)\n";
    
    BitboardPosition pos2_copy2 = pos2;
    uint64_t warm_depth2 = perft(pos2_copy2, 2);
    std::cout << "Warm depth 2: " << warm_depth2 << " (expected: 2039)\n";
    
    BitboardPosition pos2_copy3 = pos2;
    uint64_t warm_depth3 = perft(pos2_copy3, 3);
    std::cout << "Warm depth 3: " << warm_depth3 << " (expected: 97862)\n";
    
    // Now run depth 4 after warming up
    BitboardPosition pos2_copy4 = pos2;
    uint64_t warm_depth4 = perft(pos2_copy4, 4);
    std::cout << "Warm depth 4: " << warm_depth4 << "\n";
    std::cout << "Expected depth 4: " << 4085603 << "\n";
    std::cout << "Difference: " << (int64_t)warm_depth4 - 4085603 << "\n\n";
    
    // Test 3: Compare cold vs warm depth 4
    std::cout << "=== Comparison ===\n";
    std::cout << "Cold depth 4:  " << cold_depth4 << "\n";
    std::cout << "Warm depth 4:  " << warm_depth4 << "\n";
    std::cout << "Cold vs Warm:  " << (int64_t)cold_depth4 - (int64_t)warm_depth4 << "\n";
    
    if (cold_depth4 == warm_depth4) {
        std::cout << "✓ IDENTICAL: No state accumulation issue\n";
    } else {
        std::cout << "✗ DIFFERENT: State accumulation or initialization issue detected!\n";
    }
    
    // Test 4: Run multiple cold depth 4 tests
    std::cout << "\n=== Multiple Cold Tests ===\n";
    for (int i = 1; i <= 3; i++) {
        BitboardPosition pos_test;
        pos_test.set_from_fen(fen);
        uint64_t test_result = perft(pos_test, 4);
        std::cout << "Cold test " << i << ": " << test_result << "\n";
    }
    
    return 0;
}