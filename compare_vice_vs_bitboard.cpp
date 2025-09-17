#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/init.hpp"
#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>

using namespace BitboardMoveGen;

// VICE perft function
static uint64_t perft_vice(Position& pos, int depth) {
    if (depth == 0) return 1;
    S_MOVELIST list;
    generate_all_moves(pos, list);
    uint64_t nodes = 0;
    for (int i = 0; i < list.count; i++) {
        const auto& m = list.moves[i];
        if (pos.MakeMove(m) == 1) {
            nodes += perft_vice(pos, depth - 1);
            pos.TakeMove();
        }
    }
    return nodes;
}

// BitboardPosition perft function
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

uint64_t perft_bitboard(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    uint64_t total_nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        total_nodes += perft_bitboard(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }
    return total_nodes;
}

int main() {
    std::cout << "=== VICE vs BitboardPosition Comparison ===\n";
    
    // Initialize VICE
    Huginn::init();
    
    // Test position after a2a3
    std::string problem_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q1p/1PPBBPPP/R3K2R b KQkq - 0 1";
    std::cout << "Problem position: " << problem_fen << "\n\n";
    
    // Test with VICE
    Position vice_pos;
    if (!vice_pos.set_from_fen(problem_fen)) {
        std::cout << "ERROR: VICE failed to parse FEN!\n";
        return 1;
    }
    
    std::cout << "=== VICE Results ===\n";
    for (int depth = 1; depth <= 3; depth++) {
        Position vice_copy = vice_pos;
        uint64_t vice_nodes = perft_vice(vice_copy, depth);
        std::cout << "VICE depth " << depth << ": " << vice_nodes << " nodes\n";
    }
    
    // Test with BitboardPosition
    BitboardPosition bb_pos;
    if (!bb_pos.set_from_fen(problem_fen)) {
        std::cout << "ERROR: BitboardPosition failed to parse FEN!\n";
        return 1;
    }
    
    std::cout << "\n=== BitboardPosition Results ===\n";
    for (int depth = 1; depth <= 3; depth++) {
        BitboardPosition bb_copy = bb_pos;
        uint64_t bb_nodes = perft_bitboard(bb_copy, depth);
        std::cout << "BitboardPosition depth " << depth << ": " << bb_nodes << " nodes\n";
    }
    
    // Compare results
    std::cout << "\n=== Comparison ===\n";
    for (int depth = 1; depth <= 4; depth++) {
        Position vice_copy = vice_pos;
        uint64_t vice_nodes = perft_vice(vice_copy, depth);
        
        BitboardPosition bb_copy = bb_pos;
        uint64_t bb_nodes = perft_bitboard(bb_copy, depth);
        
        int64_t diff = (int64_t)bb_nodes - (int64_t)vice_nodes;
        std::cout << "Depth " << depth << ": ";
        std::cout << "VICE=" << vice_nodes << ", BB=" << bb_nodes;
        std::cout << ", Diff=" << diff;
        if (diff == 0) {
            std::cout << " ✓ MATCH\n";
        } else {
            std::cout << " ❌ MISMATCH\n";
            break; // Stop at first mismatch for detailed analysis
        }
    }
    
    // Also test the original Kiwipete position for comparison
    std::cout << "\n=== Original Kiwipete Comparison ===\n";
    const char* original_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    Position vice_orig;
    vice_orig.set_from_fen(original_fen);
    
    BitboardPosition bb_orig;
    bb_orig.set_from_fen(original_fen);
    
    for (int depth = 1; depth <= 3; depth++) {
        Position vice_copy = vice_orig;
        uint64_t vice_nodes = perft_vice(vice_copy, depth);
        
        BitboardPosition bb_copy = bb_orig;
        uint64_t bb_nodes = perft_bitboard(bb_copy, depth);
        
        int64_t diff = (int64_t)bb_nodes - (int64_t)vice_nodes;
        std::cout << "Original depth " << depth << ": ";
        std::cout << "VICE=" << vice_nodes << ", BB=" << bb_nodes;
        std::cout << ", Diff=" << diff;
        if (diff == 0) {
            std::cout << " ✓ MATCH\n";
        } else {
            std::cout << " ❌ MISMATCH\n";
        }
    }
    
    return 0;
}