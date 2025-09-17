#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/init.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace BitboardMoveGen;

// Convert BitboardMoveList::BitboardMove to SimpleBitboardMove
SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.promotion_type = move.promotion_type;
    simple_move.is_capture = move.is_capture;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    simple_move.is_promotion = move.promotion_type != PieceType::None;
    return simple_move;
}

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
static uint64_t perft_bitboard(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        nodes += perft_bitboard(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }
    return nodes;
}

// VICE divide function
void vice_divide(Position& pos, int depth) {
    S_MOVELIST list;
    generate_all_moves(pos, list);
    
    std::cout << "=== VICE Divide (depth " << depth << ") ===\n";
    uint64_t total = 0;
    
    for (int i = 0; i < list.count; i++) {
        const auto& m = list.moves[i];
        if (pos.MakeMove(m) == 1) {
            uint64_t nodes = perft_vice(pos, depth - 1);
            pos.TakeMove();
            
            std::string move_str = "[" + std::to_string(m.get_from()) + "->" + std::to_string(m.get_to()) + "]";
            std::cout << move_str << " - " << nodes << "\n";
            total += nodes;
        }
    }
    std::cout << "VICE Total: " << total << " nodes\n\n";
}

// BitboardPosition divide function
void bitboard_divide(BitboardPosition& pos, int depth) {
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    std::cout << "=== BitboardPosition Divide (depth " << depth << ") ===\n";
    uint64_t total = 0;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        uint64_t nodes = perft_bitboard(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
        
        // Print move in a simple format
        std::cout << "[" << simple_move.from_64 << "->" << simple_move.to_64 << "] - " << nodes << "\n";
        total += nodes;
    }
    std::cout << "BitboardPosition Total: " << total << " nodes\n\n";
}

int main() {
    // Initialize the engine systems
    Huginn::init();

    std::cout << "=== Debug +82 Node Difference ===\n\n";

    // Problem position after a2a3
    std::string problem_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q1p/1PPBBPPP/R3K2R b KQkq - 0 1";
    std::cout << "Position: " << problem_fen << "\n\n";

    // Test with VICE
    Position vice_pos;
    if (!vice_pos.set_from_fen(problem_fen)) {
        std::cout << "ERROR: VICE failed to parse FEN!\n";
        return 1;
    }

    // Test with BitboardPosition
    BitboardPosition bb_pos;
    if (!bb_pos.set_from_fen(problem_fen)) {
        std::cout << "ERROR: BitboardPosition failed to parse FEN!\n";
        return 1;
    }

    // Do divide analysis at depth 3 to see which moves differ
    Position vice_copy = vice_pos;
    BitboardPosition bb_copy = bb_pos;
    
    // Run basic comparison first
    std::cout << "=== Quick Depth Comparison ===\n";
    for (int d = 1; d <= 4; d++) {
        Position v = vice_pos;
        BitboardPosition b = bb_pos;
        uint64_t vice_result = perft_vice(v, d);
        uint64_t bb_result = perft_bitboard(b, d);
        std::cout << "Depth " << d << ": VICE=" << vice_result << ", BB=" << bb_result;
        if (vice_result != bb_result) {
            std::cout << " ❌ DIFF=" << (int64_t)(bb_result - vice_result) << "\n";
            break;
        } else {
            std::cout << " ✓\n";
        }
    }
    std::cout << "\n";
    
    vice_divide(vice_copy, 3);
    bitboard_divide(bb_copy, 3);

    return 0;
}