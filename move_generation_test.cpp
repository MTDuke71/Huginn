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

int main() {
    cout << "=== Move Generation Test: After a2a3 ===" << endl;
    
    // Start from position after a2a3
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    cout << "Position: rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1" << endl;
    cout << "Side to move: " << (pos.side_to_move == Color::Black ? "Black" : "White") << endl << endl;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    cout << "Generated moves: " << moves.moves.size() << endl;
    cout << "Legal moves:" << endl;
    
    int legal_count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            cout << "  " << move_to_string(move);
            if (move.is_capture) cout << " (capture)";
            if (move.is_ep_capture) cout << " (en passant)";
            if (move.is_castling) cout << " (castling)";
            if (move.is_promotion) cout << " (promotion)";
            cout << endl;
            legal_count++;
        }
    }
    
    cout << endl << "Total legal moves: " << legal_count << endl;
    cout << "Expected: 20 legal moves from this position" << endl;
    
    return 0;
}