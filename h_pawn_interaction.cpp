#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

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
    std::string result = square_to_notation(move.from_64) + square_to_notation(move.to_64);
    if (move.is_ep_capture) result += " (en passant)";
    if (move.is_castling) result += " (castle)";
    if (move.is_promotion) result += " (promotion)";
    return result;
}

void analyze_position(const std::string& description, const std::string& fen) {
    std::cout << "=== " << description << " ===\n";
    
    BitboardPosition pos;
    pos.set_from_fen(fen);
    
    std::cout << "Position: " << pos.to_fen() << "\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    int legal_count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            legal_count++;
            std::cout << move_to_notation(simple_move) << "\n";
        }
    }
    
    std::cout << "\nTotal legal moves: " << legal_count << "\n\n";
}

int main() {
    std::cout << "=== H-Pawn Interaction Analysis ===\n\n";
    
    // Position after 1.h2h4 h7h5 (both h-pawns advanced)
    analyze_position("After 1.h2h4 h7h5", 
                    "rnbqkbnr/ppppppp1/8/7p/7P/8/PPPPPPP1/RNBQKBNR w KQkq h6 0 2");
    
    // Compare with position after 1.h2h4 g7g5 (the move that gives +1 node)
    analyze_position("After 1.h2h4 g7g5", 
                    "rnbqkbnr/pppppp1p/8/6p1/7P/8/PPPPPPP1/RNBQKBNR w KQkq g6 0 2");
    
    // Compare with a normal move like 1.h2h4 e7e5
    analyze_position("After 1.h2h4 e7e5", 
                    "rnbqkbnr/pppp1ppp/8/4p3/7P/8/PPPPPPP1/RNBQKBNR w KQkq e6 0 2");
    
    return 0;
}