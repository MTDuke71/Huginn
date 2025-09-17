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

// Convert 120-square to 64-square for comparison
int square120_to_64(int sq120) {
    int rank = sq120 / 10 - 2;  // Remove border
    int file = sq120 % 10 - 1;  // Remove border
    if (rank < 0 || rank > 7 || file < 0 || file > 7) return -1;
    return rank * 8 + file;
}

int main() {
    // Initialize the engine systems
    Huginn::init();

    std::cout << "=== Find Depth 4 Bug - Move by Move Analysis ===\n\n";

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

    std::cout << "=== Move-by-Move Depth 3 Analysis ===\n";
    std::cout << "Looking for moves that give different results at depth 3...\n\n";

    // Get VICE moves
    S_MOVELIST vice_moves;
    generate_all_moves(vice_pos, vice_moves);
    
    // Get BitboardPosition moves  
    BitboardMoveList bb_moves;
    generate_legal_moves(bb_pos, bb_moves);

    std::cout << "VICE has " << vice_moves.count << " moves\n";
    std::cout << "BitboardPosition has " << bb_moves.moves.size() << " moves\n\n";

    uint64_t total_diff = 0;
    int diff_count = 0;

    // Test each VICE move
    for (int i = 0; i < vice_moves.count; i++) {
        const auto& vice_move = vice_moves.moves[i];
        
        // Make move with VICE
        Position vice_copy = vice_pos;
        if (vice_copy.MakeMove(vice_move) != 1) continue;
        
        uint64_t vice_nodes = perft_vice(vice_copy, 3);
        vice_copy.TakeMove();
        
        // Convert VICE move to 64-square format
        int from_64 = square120_to_64(vice_move.get_from());
        int to_64 = square120_to_64(vice_move.get_to());
        
        if (from_64 == -1 || to_64 == -1) continue;
        
        // Find corresponding BitboardPosition move
        uint64_t bb_nodes = 0;
        bool found_move = false;
        
        for (const auto& bb_move : bb_moves.moves) {
            if (bb_move.from_64 == from_64 && bb_move.to_64 == to_64) {
                SimpleBitboardMove simple_move = convert_move(bb_move);
                BitboardPosition bb_copy = bb_pos;
                BitboardPosition::UndoInfo undo = bb_copy.make_move_with_undo(simple_move);
                bb_nodes = perft_bitboard(bb_copy, 3);
                bb_copy.unmake_move(simple_move, undo);
                found_move = true;
                break;
            }
        }
        
        if (!found_move) {
            std::cout << "⚠️  VICE move [" << vice_move.get_from() << "->" << vice_move.get_to() 
                      << "] -> [" << from_64 << "->" << to_64 << "] not found in BitboardPosition!\n";
            continue;
        }
        
        int64_t diff = (int64_t)bb_nodes - (int64_t)vice_nodes;
        if (diff != 0) {
            std::cout << "❌ DIFFERENCE at move [" << from_64 << "->" << to_64 << "]: ";
            std::cout << "VICE=" << vice_nodes << ", BB=" << bb_nodes << ", Diff=" << diff << "\n";
            total_diff += diff;
            diff_count++;
        } else {
            std::cout << "✓ [" << from_64 << "->" << to_64 << "]: " << vice_nodes << " nodes\n";
        }
    }
    
    std::cout << "\n=== Summary ===\n";
    std::cout << "Total difference: " << total_diff << " nodes\n";
    std::cout << "Moves with differences: " << diff_count << "\n";
    
    if (total_diff == 82) {
        std::cout << "✅ Successfully isolated the +82 node bug!\n";
    } else {
        std::cout << "❓ Expected +82 but found " << total_diff << "\n";
    }

    return 0;
}