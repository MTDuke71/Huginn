#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_attacks.hpp"
#include "src/chess_types.hpp"
#include <iostream>

using namespace BitboardMoveGen;

// Simple test to check move counts
int main() {
    // Initialize attack tables
    init_knight_attacks();
    init_king_attacks();
    init_magic_bitboards();
    
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Starting position legal moves:" << std::endl;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    int legal_count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move;
        simple_move.from_64 = move.from_64;
        simple_move.to_64 = move.to_64;
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        simple_move.promotion_type = move.promotion_type;
        
        if (pos.is_legal_move(simple_move)) {
            legal_count++;
            char from_file = 'a' + file_of_64(move.from_64);
            char from_rank = '1' + rank_of_64(move.from_64);
            char to_file = 'a' + file_of_64(move.to_64);
            char to_rank = '1' + rank_of_64(move.to_64);
            std::cout << from_file << from_rank << to_file << to_rank << std::endl;
        }
    }
    
    std::cout << "\nTotal legal moves: " << legal_count << std::endl;
    
    // Test after a2a3
    std::cout << "\n\nAfter a2a3:" << std::endl;
    
    SimpleBitboardMove a2a3;
    a2a3.from_64 = 8;  // a2 = rank 1 * 8 + file 0 = 8
    a2a3.to_64 = 16;   // a3 = rank 2 * 8 + file 0 = 16
    a2a3.is_capture = false;
    a2a3.is_ep_capture = false;
    a2a3.is_castling = false;
    a2a3.is_promotion = false;
    a2a3.promotion_type = PieceType::None;
    
    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(a2a3);
    
    moves.moves.clear();
    generate_all_moves(pos, moves);
    
    legal_count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move;
        simple_move.from_64 = move.from_64;
        simple_move.to_64 = move.to_64;
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        simple_move.promotion_type = move.promotion_type;
        
        if (pos.is_legal_move(simple_move)) {
            legal_count++;
            char from_file = 'a' + file_of_64(move.from_64);
            char from_rank = '1' + rank_of_64(move.from_64);
            char to_file = 'a' + file_of_64(move.to_64);
            char to_rank = '1' + rank_of_64(move.to_64);
            std::cout << from_file << from_rank << to_file << to_rank << std::endl;
        }
    }
    
    std::cout << "\nTotal legal moves after a2a3: " << legal_count << std::endl;
    
    return 0;
}