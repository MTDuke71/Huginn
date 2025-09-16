/**
 * @file debug_original_kiwipete.cpp
 * @brief Debug what moves are available from original Kiwipete
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>

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

int main() {
    std::cout << "=== DEBUG ORIGINAL KIWIPETE MOVES ===" << std::endl;
    
    BitboardPosition pos;
    pos.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    std::cout << "Total legal moves: " << moves.moves.size() << std::endl;
    std::cout << "\nAll available moves:" << std::endl;
    
    for (size_t i = 0; i < moves.moves.size(); i++) {
        SimpleBitboardMove simple_move = convert_move(moves.moves[i]);
        std::string move_str = move_to_string(simple_move);
        std::cout << (i+1) << ". " << move_str << std::endl;
        
        if (move_str == "f2f3") {
            std::cout << "   ^^ FOUND f2f3!" << std::endl;
        }
    }
    
    std::cout << "\nLooking specifically for f-pawn moves:" << std::endl;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        if (move_str[0] == 'f' || move_str[2] == 'f') {
            std::cout << "F-pawn move: " << move_str << std::endl;
        }
    }
    
    return 0;
}