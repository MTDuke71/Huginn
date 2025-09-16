#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

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

// Convert 64-square to algebraic notation
string square64_to_algebraic(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return string(1, 'a' + file) + string(1, '1' + rank);
}

// Convert move to string
string move_to_string(const BitboardMoveList::BitboardMove& move) {
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

int main() {
    cout << "=== Depth 3 to 4 Transition Analysis ===" << endl;
    
    // Start from position after a2a3
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    cout << "Position: rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1" << endl;
    cout << "Testing each depth 1 move to see depth 3 perft results..." << endl << endl;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t total_depth3 = 0;
    
    cout << "Move     Depth 3 Perft   Notes" << endl;
    cout << "---------------------------------------" << endl;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            
            uint64_t depth3_result = perft_legal(pos, 3);
            total_depth3 += depth3_result;
            
            string move_str = move_to_string(move);
            cout << left << setw(8) << move_str 
                 << " " << setw(12) << depth3_result;
            
            // Flag unusually low results
            if (depth3_result < 100) {
                cout << " ⚠️ LOW";
            } else if (depth3_result > 1000) {
                cout << " 📈 HIGH";
            }
            cout << endl;
            
            pos.unmake_move(simple_move, undo);
        }
    }
    
    cout << "---------------------------------------" << endl;
    cout << "Total depth 4 (sum): " << total_depth3 << endl;
    cout << "Expected around:      ~170,000-200,000" << endl;
    cout << "Our depth 4 direct:   181,046" << endl;
    
    return 0;
}