#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;
using namespace std;

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

// Perft function 
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

// Convert 64-square to algebraic notation
string square64_to_algebraic(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return string(1, 'a' + file) + string(1, '1' + rank);
}

// Convert move to string
string move_to_string_detailed(const BitboardMoveList::BitboardMove& move) {
    string result = square64_to_algebraic(move.from_64) + square64_to_algebraic(move.to_64);
    if (move.is_promotion) {
        switch (move.promotion_type) {
            case PieceType::Queen: result += "q"; break;
            case PieceType::Rook: result += "r"; break;
            case PieceType::Bishop: result += "b"; break;
            case PieceType::Knight: result += "n"; break;
            default: break;
        }
    }
    return result;
}

int main() {
    cout << "=== Perft Verification: a2a3 a7a5 vs a2a3 a7a6 ===" << endl;
    
    // === Test with a7a5 (creates en passant target) ===
    BitboardPosition pos1;
    pos1.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    // Make a7a5
    BitboardMoveList moves1;
    generate_all_moves(pos1, moves1);
    for (const auto& move : moves1.moves) {
        if (move_to_string_detailed(move) == "a7a5") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos1.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo1 = pos1.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    cout << "After a2a3 a7a5:" << endl;
    uint64_t perft1_depth1 = perft_legal(pos1, 1);
    cout << "Depth 1 perft: " << perft1_depth1 << endl;
    uint64_t perft1_depth2 = perft_legal(pos1, 2);
    cout << "Depth 2 perft: " << perft1_depth2 << endl;
    
    // === Test with a7a6 (no en passant target) ===
    BitboardPosition pos2;
    pos2.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    // Make a7a6
    BitboardMoveList moves2;
    generate_all_moves(pos2, moves2);
    for (const auto& move : moves2.moves) {
        if (move_to_string_detailed(move) == "a7a6") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos2.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo2 = pos2.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    cout << "\nAfter a2a3 a7a6:" << endl;
    uint64_t perft2_depth1 = perft_legal(pos2, 1);
    cout << "Depth 1 perft: " << perft2_depth1 << endl;
    uint64_t perft2_depth2 = perft_legal(pos2, 2);
    cout << "Depth 2 perft: " << perft2_depth2 << endl;
    
    cout << "\nDifferences:" << endl;
    cout << "Depth 1: a7a5 has " << (int64_t)perft1_depth1 - (int64_t)perft2_depth1 << " more nodes" << endl;
    cout << "Depth 2: a7a5 has " << (int64_t)perft1_depth2 - (int64_t)perft2_depth2 << " more nodes" << endl;
    
    return 0;
}