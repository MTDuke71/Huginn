#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/init.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <map>

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

void deep_analyze_move(const std::string& move_name, int from_sq, int to_sq, 
                      Position& vice_pos, BitboardPosition& bb_pos) {
    std::cout << "\n=== Deep Analysis: " << move_name << " (" << sq64_to_alg(from_sq) << sq64_to_alg(to_sq) << ") ===\n";
    
    // Find and make VICE move
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
    
    // Find and make BitboardPosition move
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
    
    // Test perft at depth 2
    Position v_test = vice_after_move;
    BitboardPosition b_test = bb_after_move;
    
    uint64_t vice_depth2 = perft_vice(v_test, 2);
    uint64_t bb_depth2 = perft_bitboard(b_test, 2);
    
    std::cout << "Depth 2: VICE=" << vice_depth2 << ", BB=" << bb_depth2;
    if (vice_depth2 != bb_depth2) {
        std::cout << " ❌ DIFF=" << ((int64_t)bb_depth2 - (int64_t)vice_depth2) << " <-- THE BUG!\n";
        
        // If there's a difference at depth 2, do a divide analysis to find which submoves cause it
        std::cout << "\n--- Divide Analysis (finding +2 source moves) ---\n";
        
        S_MOVELIST vice_submoves;
        generate_all_moves(vice_after_move, vice_submoves);
        
        BitboardMoveList bb_submoves;
        generate_legal_moves(bb_after_move, bb_submoves);
        
        // Create move comparison map
        std::map<std::string, std::pair<uint64_t, uint64_t>> move_results;
        
        // Test each VICE move
        for (int i = 0; i < vice_submoves.count; i++) {
            const auto& vmove = vice_submoves.moves[i];
            Position v_copy = vice_after_move;
            
            if (v_copy.MakeMove(vmove) == 1) {
                uint64_t v_nodes = perft_vice(v_copy, 1);
                v_copy.TakeMove();
                
                int v_from = square120_to_64(vmove.get_from());
                int v_to = square120_to_64(vmove.get_to());
                std::string v_notation = sq64_to_alg(v_from) + sq64_to_alg(v_to);
                
                move_results[v_notation].first = v_nodes;
            }
        }
        
        // Test each BitboardPosition move
        for (const auto& bmove : bb_submoves.moves) {
            SimpleBitboardMove simple_move = convert_move(bmove);
            BitboardPosition b_copy = bb_after_move;
            
            BitboardPosition::UndoInfo undo = b_copy.make_move_with_undo(simple_move);
            uint64_t b_nodes = perft_bitboard(b_copy, 1);
            b_copy.unmake_move(simple_move, undo);
            
            std::string b_notation = sq64_to_alg(bmove.from_64) + sq64_to_alg(bmove.to_64);
            move_results[b_notation].second = b_nodes;
        }
        
        // Display results
        int problematic_moves = 0;
        for (const auto& [move_notation, results] : move_results) {
            uint64_t vice_nodes = results.first;
            uint64_t bb_nodes = results.second;
            
            if (vice_nodes != bb_nodes) {
                std::cout << move_notation << ": VICE=" << vice_nodes << ", BB=" << bb_nodes 
                         << " DIFF=" << ((int64_t)bb_nodes - (int64_t)vice_nodes) << " ❌\n";
                problematic_moves++;
            }
        }
        
        std::cout << "Total problematic submoves: " << problematic_moves << "\n";
        
    } else {
        std::cout << " ✓ No difference at depth 2\n";
    }
}

int main() {
    Huginn::init();

    std::cout << "=== Deep Analysis of +2 Systematic Overcounting ===\n\n";

    // Position after h7->g6
    std::string after_h7g6_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q2/1PPBBPpP/R3K2R w KQkq - 0 2";
    std::cout << "Position: " << after_h7g6_fen << "\n";

    Position vice_pos;
    vice_pos.set_from_fen(after_h7g6_fen);

    BitboardPosition bb_pos;
    bb_pos.set_from_fen(after_h7g6_fen);

    // Analyze problematic move (known +2 error)
    deep_analyze_move("PROBLEMATIC b2b3", 9, 17, vice_pos, bb_pos);
    
    // Analyze accurate move (known no error)  
    deep_analyze_move("ACCURATE e2f1", 12, 5, vice_pos, bb_pos);

    return 0;
}