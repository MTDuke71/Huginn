#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <vector>

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

std::string square_to_notation(int square) {
    int file = square % 8;
    int rank = square / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

std::string move_to_notation(const SimpleBitboardMove& move) {
    return square_to_notation(move.from_64) + square_to_notation(move.to_64);
}

// Simple perft that just counts
uint64_t simple_perft(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            count += simple_perft(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
        }
    }
    
    return count;
}

// Perft divide - shows count for each move
void perft_divide(BitboardPosition& pos, int depth) {
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << "=== Black's Legal Moves After h2h4 ===\n";
    std::cout << "Position: " << pos.to_fen() << "\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n\n";
    
    uint64_t total = 0;
    int move_count = 0;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            move_count++;
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t count = simple_perft(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
            
            std::cout << move_to_notation(simple_move) << ": " << count << "\n";
            total += count;
        }
    }
    
    std::cout << "\nTotal moves: " << move_count << "\n";
    std::cout << "Total nodes: " << total << "\n";
    std::cout << "Expected: 421 nodes\n";
    std::cout << "Difference: " << (int64_t)total - 421 << "\n";
}

int main() {
    std::cout << "=== Corrected H2H4 Perft Analysis ===\n\n";
    
    // Set up position after h2h4
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/7P/8/PPPPPPP1/RNBQKBNR b KQkq h3 0 1");
    
    perft_divide(pos, 2);
    
    std::cout << "\n=== Standard Black Opening Moves ===\n";
    std::cout << "Expected 20 moves:\n";
    std::cout << "Pawns: a7a6, a7a5, b7b6, b7b5, c7c6, c7c5, d7d6, d7d5\n";
    std::cout << "       e7e6, e7e5, f7f6, f7f5, g7g6, g7g5, h7h6, h7h5\n";
    std::cout << "Knights: b8a6, b8c6, g8f6, g8h6\n";
    
    return 0;
}