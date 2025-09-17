#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>
#include <map>
#include <set>

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

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

uint64_t perft_with_move_counting(BitboardPosition& pos, int depth, std::map<std::string, uint64_t>& move_counts) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        uint64_t subtree_nodes = perft_with_move_counting(pos, depth - 1, move_counts);
        pos.unmake_move(simple_move, undo);
        
        if (depth == 1) {
            // Count leaf nodes by move
            move_counts[move_str] += subtree_nodes;
        }
        
        total_nodes += subtree_nodes;
    }

    return total_nodes;
}

uint64_t simple_perft(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    uint64_t total_nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        total_nodes += simple_perft(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }
    return total_nodes;
}

int main() {
    std::cout << "=== Move Generation Analysis ===\n";
    
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    BitboardPosition pos;
    pos.set_from_fen(fen);
    
    std::cout << "FEN: " << fen << "\n\n";
    
    // Test if we get consistent move counts at different calls
    std::cout << "=== Consistency Test ===\n";
    for (int test = 1; test <= 3; test++) {
        BitboardPosition test_pos;
        test_pos.set_from_fen(fen);
        
        BitboardMoveList moves;
        generate_legal_moves(test_pos, moves);
        
        std::cout << "Test " << test << ": " << moves.moves.size() << " legal moves generated\n";
    }
    
    // Count moves at depth 1 vs depth 2
    std::cout << "\n=== Depth Comparison ===\n";
    
    BitboardPosition pos1 = pos;
    uint64_t depth1_total = simple_perft(pos1, 1);
    std::cout << "Depth 1 total: " << depth1_total << " (expected: 48)\n";
    
    BitboardPosition pos2 = pos;
    uint64_t depth2_total = simple_perft(pos2, 2);
    std::cout << "Depth 2 total: " << depth2_total << " (expected: 2039)\n";
    
    // Detailed depth 2 analysis with move counting
    std::cout << "\n=== Detailed Depth 2 Analysis ===\n";
    std::map<std::string, uint64_t> depth2_move_counts;
    BitboardPosition pos_detailed = pos;
    uint64_t depth2_detailed = perft_with_move_counting(pos_detailed, 2, depth2_move_counts);
    
    std::cout << "Depth 2 detailed total: " << depth2_detailed << "\n";
    std::cout << "Move count breakdown (showing first 10):\n";
    
    int count = 0;
    for (const auto& pair : depth2_move_counts) {
        if (count >= 10) break;
        std::cout << "  " << pair.first << ": " << pair.second << " nodes\n";
        count++;
    }
    
    // Check if any moves appear multiple times
    std::cout << "\n=== Duplicate Move Detection ===\n";
    BitboardMoveList all_moves;
    generate_legal_moves(pos, all_moves);
    
    std::set<std::string> seen_moves;
    std::set<std::string> duplicate_moves;
    
    for (const auto& move : all_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        if (seen_moves.count(move_str)) {
            duplicate_moves.insert(move_str);
        } else {
            seen_moves.insert(move_str);
        }
    }
    
    if (duplicate_moves.empty()) {
        std::cout << "✓ No duplicate moves detected in move generation\n";
    } else {
        std::cout << "❌ Duplicate moves detected:\n";
        for (const auto& move : duplicate_moves) {
            std::cout << "  " << move << "\n";
        }
    }
    
    // Test the problematic moves specifically
    std::cout << "\n=== Testing Problematic Knight Moves ===\n";
    
    // Test e5f7
    SimpleBitboardMove e5f7;
    e5f7.from_64 = 36; e5f7.to_64 = 53; e5f7.is_capture = true;
    e5f7.is_ep_capture = false; e5f7.is_castling = false; e5f7.is_promotion = false;
    
    BitboardPosition::UndoInfo undo_e5f7 = pos.make_move_with_undo(e5f7);
    uint64_t e5f7_depth3 = simple_perft(pos, 3);
    pos.unmake_move(e5f7, undo_e5f7);
    
    std::cout << "e5f7 depth 3 nodes: " << e5f7_depth3 << "\n";
    
    // Test e5g6
    SimpleBitboardMove e5g6;
    e5g6.from_64 = 36; e5g6.to_64 = 46; e5g6.is_capture = true;
    e5g6.is_ep_capture = false; e5g6.is_castling = false; e5g6.is_promotion = false;
    
    BitboardPosition::UndoInfo undo_e5g6 = pos.make_move_with_undo(e5g6);
    uint64_t e5g6_depth3 = simple_perft(pos, 3);
    pos.unmake_move(e5g6, undo_e5g6);
    
    std::cout << "e5g6 depth 3 nodes: " << e5g6_depth3 << "\n";
    
    return 0;
}