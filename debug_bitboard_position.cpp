/**
 * @file debug_bitboard_position.cpp
 * @brief Diagnostic tool to find bugs in BitboardPosition::make_move_with_undo
 * 
 * This program compares BitboardPosition against the trusted Position class
 * to identify exactly where the move-making logic diverges, causing systematic
 * undercounting for slider pieces (rooks, bishops, queen) and king moves.
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include <iostream>
#include <iomanip>

using namespace BitboardMoveGen;

std::string bb_move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

std::string s_move_to_string(const S_MOVE& move) {
    int from = move.get_from();
    int to = move.get_to();
    
    // Convert 120-square to algebraic
    char from_file = 'a' + ((from % 10) - 1);
    char from_rank = '1' + ((from / 10) - 2);
    char to_file = 'a' + ((to % 10) - 1);
    char to_rank = '1' + ((to / 10) - 2);
    
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

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

void print_position_comparison(BitboardPosition& bb_pos, Position& pos_120, const std::string& label) {
    std::cout << "\n=== " << label << " ===" << std::endl;
    
    // Compare basic state
    std::cout << "Side to move: BB=" << (bb_pos.side_to_move == Color::White ? "White" : "Black")
              << " vs 120=" << (pos_120.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << "Castling rights: BB=" << int(bb_pos.castling_rights) 
              << " vs 120=" << int(pos_120.castling_rights) << std::endl;
    std::cout << "En passant: BB=" << bb_pos.ep_square_64 
              << " vs 120=" << pos_120.ep_square << std::endl;
    std::cout << "Halfmove: BB=" << bb_pos.halfmove_clock 
              << " vs 120=" << pos_120.halfmove_clock << std::endl;
    std::cout << "Ply: BB=" << bb_pos.ply 
              << " vs 120=" << pos_120.ply << std::endl;
    
    // Generate moves for comparison
    BitboardMoveList bb_moves;
    generate_legal_moves(bb_pos, bb_moves);
    
    S_MOVELIST moves_120;
    generate_all_moves(pos_120, moves_120);
    
    std::cout << "Legal moves: BB=" << bb_moves.moves.size() 
              << " vs 120=" << moves_120.count << std::endl;
    
    if (bb_moves.moves.size() != moves_120.count) {
        std::cout << "WARNING: Move count mismatch!" << std::endl;
    }
}

uint64_t perft_bitboard(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        total += perft_bitboard(pos, depth - 1);
        pos.unmake_move(simple_move, undo);
    }
    return total;
}

uint64_t perft_120(Position& pos, int depth) {
    if (depth == 0) return 1;

    S_MOVELIST moves;
    generate_all_moves(pos, moves);

    uint64_t total = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.MakeMove(moves.moves[i]) == 1) {
            total += perft_120(pos, depth - 1);
            pos.TakeMove();
        }
    }
    return total;
}

void test_specific_move(const std::string& fen, const std::string& move_str) {
    std::cout << "\n=== TESTING MOVE " << move_str << " FROM " << fen << " ===" << std::endl;
    
    // Set up both position types
    BitboardPosition bb_pos;
    Position pos_120;
    
    if (!bb_pos.set_from_fen(fen)) {
        std::cout << "Failed to set BitboardPosition from FEN" << std::endl;
        return;
    }
    
    if (!pos_120.set_from_fen(fen)) {
        std::cout << "Failed to set Position from FEN" << std::endl;
        return;
    }
    
    print_position_comparison(bb_pos, pos_120, "BEFORE MOVE");
    
    // Find the move in both systems
    BitboardMoveList bb_moves;
    generate_legal_moves(bb_pos, bb_moves);
    
    S_MOVELIST moves_120;
    generate_all_moves(pos_120, moves_120);
    
    // Find matching moves
    SimpleBitboardMove target_bb_move;
    S_MOVE target_120_move;
    bool found_bb = false, found_120 = false;
    
    // Search BitboardPosition moves
    for (const auto& move : bb_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_string = bb_move_to_string(simple_move);
        
        if (move_string == move_str) {
            target_bb_move = simple_move;
            found_bb = true;
            break;
        }
    }
    
    // Search Position moves
    for (int i = 0; i < moves_120.count; i++) {
        std::string move_string = s_move_to_string(moves_120.moves[i]);
        if (move_string == move_str) {
            target_120_move = moves_120.moves[i];
            found_120 = true;
            break;
        }
    }
    
    if (!found_bb || !found_120) {
        std::cout << "Move not found: BB=" << found_bb << " 120=" << found_120 << std::endl;
        return;
    }
    
    // Make the moves
    BitboardPosition::UndoInfo bb_undo = bb_pos.make_move_with_undo(target_bb_move);
    int result_120 = pos_120.MakeMove(target_120_move);
    
    if (result_120 != 1) {
        std::cout << "120-square move failed!" << std::endl;
        return;
    }
    
    print_position_comparison(bb_pos, pos_120, "AFTER MOVE");
    
    // Test perft from resulting positions
    std::cout << "\nPerft comparison:" << std::endl;
    for (int depth = 1; depth <= 3; depth++) {
        // Save positions for multiple perft calls
        BitboardPosition bb_copy = bb_pos;
        Position pos_120_copy = pos_120;
        
        uint64_t bb_nodes = perft_bitboard(bb_copy, depth);
        uint64_t nodes_120 = perft_120(pos_120_copy, depth);
        
        std::cout << "Depth " << depth << ": BB=" << bb_nodes 
                  << " vs 120=" << nodes_120;
        if (bb_nodes != nodes_120) {
            std::cout << " MISMATCH! Diff=" << int64_t(bb_nodes) - int64_t(nodes_120);
        }
        std::cout << std::endl;
    }
    
    // Undo moves
    bb_pos.unmake_move(target_bb_move, bb_undo);
    pos_120.TakeMove();
    
    print_position_comparison(bb_pos, pos_120, "AFTER UNDO");
}

int main() {
    std::cout << "=== BITBOARDPOSITION DEBUG TOOL ===" << std::endl;
    
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    // Test known problematic moves
    std::vector<std::string> problem_moves = {
        "f3f5",  // Queen move with largest deficit (-14718)
        "d2e3",  // Bishop move with large deficit
        "a1b1",  // Rook move with deficit
        "e1g1",  // King move (castling)
        "c3a4",  // Knight move (should work correctly)
        "a2a3"   // Pawn move (should work correctly)
    };
    
    for (const std::string& move : problem_moves) {
        test_specific_move(kiwipete_fen, move);
        std::cout << std::string(80, '=') << std::endl;
    }
    
    return 0;
}