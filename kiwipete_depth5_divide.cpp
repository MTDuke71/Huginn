/**
 * @file kiwipete_depth5_divide.cpp
 * @brief Perft divide at depth 5 for Kiwipete position to find missing nodes
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>

using namespace BitboardMoveGen;

// Helper function to convert move for compatibility
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

// Simple perft function
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

// Get piece type at square for move classification
char get_piece_type(const std::string& move_str) {
    // Look at the from square to determine piece type
    // This is a simplified classification based on common patterns
    if (move_str[1] == '2' || move_str[1] == '7') return 'P'; // Pawn moves
    // For this position, we can classify based on known piece positions
    char from_file = move_str[0];
    char from_rank = move_str[1];
    
    // Known pieces in Kiwipete starting position
    if (from_file == 'c' && from_rank == '3') return 'N'; // Knight on c3
    if (from_file == 'e' && from_rank == '5') return 'N'; // Knight on e5  
    if (from_file == 'f' && from_rank == '3') return 'Q'; // Queen on f3
    if (from_file == 'e' && from_rank == '1') return 'K'; // King on e1
    if (from_file == 'a' && from_rank == '1') return 'R'; // Rook on a1
    if (from_file == 'h' && from_rank == '1') return 'R'; // Rook on h1
    if (from_file == 'd' && from_rank == '2') return 'B'; // Bishop on d2
    if (from_file == 'e' && from_rank == '2') return 'B'; // Bishop on e2
    
    return '?'; // Unknown
}

int main() {
    std::cout << "=== KIWIPETE DEPTH 5 PERFT DIVIDE ===" << std::endl;
    
    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "FEN: " << fen << std::endl;
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cerr << "Failed to parse FEN!" << std::endl;
        return 1;
    }
    
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    std::cout << "\nTotal moves from root: " << moves.moves.size() << std::endl;
    std::cout << "\n=== PERFT DIVIDE DEPTH 5 ===" << std::endl;
    
    uint64_t total_nodes = 0;
    std::map<char, uint64_t> piece_totals;
    std::map<char, int> piece_counts;
    
    std::cout << "Move | Piece | Depth 5 Nodes" << std::endl;
    std::cout << "-----|-------|---------------" << std::endl;
    
    for (size_t i = 0; i < moves.moves.size(); i++) {
        SimpleBitboardMove simple_move = convert_move(moves.moves[i]);
        std::string move_str = move_to_string(simple_move);
        char piece_type = get_piece_type(move_str);
        
        BitboardPosition pos_copy = pos;
        BitboardPosition::UndoInfo undo = pos_copy.make_move_with_undo(simple_move);
        uint64_t nodes = perft(pos_copy, 4);  // Depth 5 total = depth 4 after move
        pos_copy.unmake_move(simple_move, undo);
        
        total_nodes += nodes;
        piece_totals[piece_type] += nodes;
        piece_counts[piece_type]++;
        
        std::cout << std::setw(4) << move_str 
                  << " |   " << piece_type 
                  << "   | " << std::setw(11) << nodes << std::endl;
    }
    
    std::cout << "\n=== SUMMARY ===" << std::endl;
    std::cout << "Total nodes: " << total_nodes << std::endl;
    std::cout << "Expected:    193690690" << std::endl;
    std::cout << "Difference:  " << (int64_t)total_nodes - 193690690 << std::endl;
    
    std::cout << "\n=== BY PIECE TYPE ===" << std::endl;
    std::cout << "Piece | Count | Total Nodes | Avg per Move" << std::endl;
    std::cout << "------|-------|-------------|-------------" << std::endl;
    
    for (const auto& [piece, total] : piece_totals) {
        int count = piece_counts[piece];
        uint64_t avg = (count > 0) ? total / count : 0;
        std::cout << "  " << piece << "   |  " << std::setw(3) << count 
                  << "  | " << std::setw(11) << total 
                  << " | " << std::setw(11) << avg << std::endl;
    }
    
    std::cout << "\nPattern Analysis:" << std::endl;
    std::cout << "- Pawns (P): " << piece_totals['P'] << " nodes from " << piece_counts['P'] << " moves" << std::endl;
    std::cout << "- Knights (N): " << piece_totals['N'] << " nodes from " << piece_counts['N'] << " moves" << std::endl;
    std::cout << "- Other pieces: " << (total_nodes - piece_totals['P'] - piece_totals['N']) << " nodes" << std::endl;
    
    return 0;
}