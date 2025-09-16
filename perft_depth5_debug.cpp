#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <map>

using namespace BitboardMoveGen;

// Convert function
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

// Convert square to algebraic notation
std::string square_to_algebraic(int square) {
    int file = square % 8;
    int rank = square / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

// Perft function with move counting
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            nodes += perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
        }
    }
    
    return nodes;
}

// Perft divide at specific depth with detailed counting
void perft_divide_detailed(BitboardPosition& pos, int depth) {
    std::cout << "=== Perft Divide " << depth << " Analysis ===\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t total_nodes = 0;
    std::map<std::string, uint64_t> move_counts;
    
    std::cout << "Analyzing " << moves.moves.size() << " pseudo-legal moves:\n\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = square_to_algebraic(simple_move.from_64) + 
                                   square_to_algebraic(simple_move.to_64);
            
            if (simple_move.is_promotion) {
                char promo_char = 'q';
                if (simple_move.promotion_type == PieceType::Rook) promo_char = 'r';
                else if (simple_move.promotion_type == PieceType::Bishop) promo_char = 'b';
                else if (simple_move.promotion_type == PieceType::Knight) promo_char = 'n';
                move_str += promo_char;
            }
            
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t subnodes = perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
            
            move_counts[move_str] = subnodes;
            total_nodes += subnodes;
            
            std::cout << move_str << ": " << subnodes;
            if (simple_move.is_capture) std::cout << " [capture]";
            if (simple_move.is_ep_capture) std::cout << " [en passant]";
            if (simple_move.is_castling) std::cout << " [castling]";
            if (simple_move.is_promotion) std::cout << " [promotion]";
            std::cout << "\n";
        }
    }
    
    std::cout << "\nTotal: " << total_nodes << " nodes\n";
    std::cout << "Expected at depth " << depth << ":\n";
    if (depth == 3) std::cout << "Expected: 8902\n";
    else if (depth == 4) std::cout << "Expected: 197281\n";
    else if (depth == 5) std::cout << "Expected: 4865609\n";
    
    std::cout << "Difference: " << (int64_t)total_nodes - (depth == 3 ? 8902 : depth == 4 ? 197281 : depth == 5 ? 4865609 : 0) << "\n";
}

int main() {
    std::cout << "=== Depth 5 Perft Debug Analysis ===\n\n";
    
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Position: " << pos.to_fen() << "\n\n";
    
    // Test depths 4 and 5 to compare
    std::cout << "First, let's verify depth 4 (should be perfect):\n";
    perft_divide_detailed(pos, 4);
    
    std::cout << "\n" << std::string(80, '=') << "\n\n";
    
    std::cout << "Now testing depth 5 (where the error occurs):\n";
    perft_divide_detailed(pos, 5);
    
    return 0;
}