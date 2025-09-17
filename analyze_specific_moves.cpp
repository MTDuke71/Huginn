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

// Convert 64-square to algebraic notation
std::string sq64_to_alg(int sq64) {
    if (sq64 < 0 || sq64 > 63) return "??";
    char file = 'a' + (sq64 % 8);
    char rank = '1' + (sq64 / 8);
    return std::string(1, file) + std::string(1, rank);
}

void analyze_move(const std::string& move_name, int from_sq, int to_sq, 
                  Position& vice_pos, BitboardPosition& bb_pos) {
    std::cout << "\n=== Analyzing " << move_name << " (" << sq64_to_alg(from_sq) << sq64_to_alg(to_sq) << ") ===\n";
    
    // Find VICE move
    S_MOVELIST vice_moves;
    generate_all_moves(vice_pos, vice_moves);
    
    Position vice_after_move;
    bool vice_found = false;
    for (int i = 0; i < vice_moves.count; i++) {
        const auto& move = vice_moves.moves[i];
        int from_64 = square120_to_64(move.get_from());
        int to_64 = square120_to_64(move.get_to());
        
        if (from_64 == from_sq && to_64 == to_sq) {
            vice_after_move = vice_pos;
            if (vice_after_move.MakeMove(move) == 1) {
                vice_found = true;
                break;
            }
        }
    }
    
    if (!vice_found) {
        std::cout << "❌ Could not find move in VICE\n";
        return;
    }
    
    // Find BitboardPosition move
    BitboardMoveList bb_moves;
    generate_legal_moves(bb_pos, bb_moves);
    
    BitboardPosition bb_after_move = bb_pos;
    bool bb_found = false;
    BitboardPosition::UndoInfo undo_info;
    
    for (const auto& move : bb_moves.moves) {
        if (move.from_64 == from_sq && move.to_64 == to_sq) {
            SimpleBitboardMove simple_move = convert_move(move);
            undo_info = bb_after_move.make_move_with_undo(simple_move);
            bb_found = true;
            break;
        }
    }
    
    if (!bb_found) {
        std::cout << "❌ Could not find move in BitboardPosition\n";
        return;
    }
    
    // Compare resulting positions
    std::cout << "VICE FEN:  " << vice_after_move.to_fen() << "\n";
    std::cout << "BB FEN:    " << bb_after_move.to_fen() << "\n";
    std::cout << "FEN Match: " << (vice_after_move.to_fen() == bb_after_move.to_fen() ? "✓" : "❌") << "\n";
    
    // Get move counts
    S_MOVELIST vice_next_moves;
    generate_all_moves(vice_after_move, vice_next_moves);
    
    BitboardMoveList bb_next_moves;
    generate_legal_moves(bb_after_move, bb_next_moves);
    
    int vice_legal_count = 0;
    for (int i = 0; i < vice_next_moves.count; i++) {
        Position test_pos = vice_after_move;
        if (test_pos.MakeMove(vice_next_moves.moves[i]) == 1) {
            vice_legal_count++;
            test_pos.TakeMove();
        }
    }
    
    std::cout << "VICE legal moves: " << vice_legal_count << "\n";
    std::cout << "BB legal moves:   " << bb_next_moves.moves.size() << "\n";
    std::cout << "Move count diff:  " << ((int)bb_next_moves.moves.size() - vice_legal_count) << "\n";
    
    // Test perft at depth 1
    Position v_test = vice_after_move;
    BitboardPosition b_test = bb_after_move;
    
    uint64_t vice_depth1 = perft_vice(v_test, 1);
    uint64_t bb_depth1 = perft_bitboard(b_test, 1);
    
    std::cout << "Depth 1: VICE=" << vice_depth1 << ", BB=" << bb_depth1;
    if (vice_depth1 != bb_depth1) {
        std::cout << " ❌ DIFF=" << ((int64_t)bb_depth1 - (int64_t)vice_depth1);
    } else {
        std::cout << " ✓";
    }
    std::cout << "\n";
}

int main() {
    Huginn::init();

    std::cout << "=== Compare Problematic vs Accurate Moves ===\n\n";

    // Position after h7->g6
    std::string after_h7g6_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q2/1PPBBPpP/R3K2R w KQkq - 0 2";
    std::cout << "Position: " << after_h7g6_fen << "\n";

    // Test with VICE
    Position vice_pos;
    vice_pos.set_from_fen(after_h7g6_fen);

    // Test with BitboardPosition
    BitboardPosition bb_pos;
    bb_pos.set_from_fen(after_h7g6_fen);

    // Analyze a problematic move (b2b3 has +2 error)
    analyze_move("PROBLEMATIC b2b3", 9, 17, vice_pos, bb_pos);
    
    // Analyze an accurate move (e2f1 has no error)
    analyze_move("ACCURATE e2f1", 12, 5, vice_pos, bb_pos);
    
    // Analyze another accurate move (e1c1 castling)
    analyze_move("ACCURATE e1c1", 4, 2, vice_pos, bb_pos);

    return 0;
}