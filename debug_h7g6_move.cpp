#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/init.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

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

// Convert 120-square to 64-square
int square120_to_64(int sq120) {
    int rank = sq120 / 10 - 2;
    int file = sq120 % 10 - 1;
    if (rank < 0 || rank > 7 || file < 0 || file > 7) return -1;
    return rank * 8 + file;
}

int main() {
    Huginn::init();

    std::cout << "=== Debug h7->g6 Move (+82 Node Bug) ===\n\n";

    // Problem position after a2a3
    std::string problem_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q1p/1PPBBPPP/R3K2R b KQkq - 0 1";
    std::cout << "Starting position: " << problem_fen << "\n\n";

    // Test with VICE
    Position vice_pos;
    vice_pos.set_from_fen(problem_fen);

    // Test with BitboardPosition
    BitboardPosition bb_pos;
    bb_pos.set_from_fen(problem_fen);

    // The problematic move: h7->g6 (23->14 in 64-square)
    std::cout << "=== Making h7->g6 Move ===\n";
    
    // Make move with VICE (need to find the 120-square equivalent)
    S_MOVELIST vice_moves;
    generate_all_moves(vice_pos, vice_moves);
    
    Position vice_after;
    bool vice_found = false;
    for (int i = 0; i < vice_moves.count; i++) {
        const auto& move = vice_moves.moves[i];
        int from_64 = square120_to_64(move.get_from());
        int to_64 = square120_to_64(move.get_to());
        
        if (from_64 == 23 && to_64 == 14) {  // h7->g6
            vice_after = vice_pos;
            if (vice_after.MakeMove(move) == 1) {
                vice_found = true;
                std::cout << "✓ VICE made h7->g6 successfully\n";
            }
            break;
        }
    }
    
    if (!vice_found) {
        std::cout << "❌ Could not find h7->g6 move in VICE\n";
        return 1;
    }

    // Make move with BitboardPosition
    BitboardMoveList bb_moves;
    generate_legal_moves(bb_pos, bb_moves);
    
    BitboardPosition bb_after = bb_pos;
    bool bb_found = false;
    BitboardPosition::UndoInfo undo_info;
    
    for (const auto& move : bb_moves.moves) {
        if (move.from_64 == 23 && move.to_64 == 14) {  // h7->g6
            SimpleBitboardMove simple_move = convert_move(move);
            undo_info = bb_after.make_move_with_undo(simple_move);
            bb_found = true;
            std::cout << "✓ BitboardPosition made h7->g6 successfully\n";
            break;
        }
    }
    
    if (!bb_found) {
        std::cout << "❌ Could not find h7->g6 move in BitboardPosition\n";
        return 1;
    }

    // Compare resulting positions
    std::cout << "\n=== Comparing Resulting Positions ===\n";
    std::cout << "VICE FEN:    " << vice_after.to_fen() << "\n";
    std::cout << "BB FEN:      " << bb_after.to_fen() << "\n";
    
    bool fen_match = (vice_after.to_fen() == bb_after.to_fen());
    std::cout << "FEN Match:   " << (fen_match ? "✓ YES" : "❌ NO") << "\n\n";

    // Compare move counts
    S_MOVELIST vice_moves_after;
    generate_all_moves(vice_after, vice_moves_after);
    
    BitboardMoveList bb_moves_after;
    generate_legal_moves(bb_after, bb_moves_after);
    
    std::cout << "VICE moves:  " << vice_moves_after.count << "\n";
    std::cout << "BB moves:    " << bb_moves_after.moves.size() << "\n";
    
    bool move_count_match = (vice_moves_after.count == bb_moves_after.moves.size());
    std::cout << "Move counts: " << (move_count_match ? "✓ YES" : "❌ NO") << "\n\n";

    // Run perft from resulting positions
    std::cout << "=== Perft from Resulting Positions ===\n";
    
    for (int depth = 1; depth <= 3; depth++) {
        Position v_copy = vice_after;
        BitboardPosition b_copy = bb_after;
        
        uint64_t vice_nodes = perft_vice(v_copy, depth);
        uint64_t bb_nodes = perft_bitboard(b_copy, depth);
        int64_t diff = (int64_t)bb_nodes - (int64_t)vice_nodes;
        
        std::cout << "Depth " << depth << ": VICE=" << vice_nodes << ", BB=" << bb_nodes;
        if (diff != 0) {
            std::cout << " ❌ DIFF=" << diff;
        } else {
            std::cout << " ✓";
        }
        std::cout << "\n";
        
        if (depth == 3 && diff == 82) {
            std::cout << "✅ Found the exact +82 difference at depth 3!\n";
        }
    }

    return 0;
}