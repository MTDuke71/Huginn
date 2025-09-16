#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;
using namespace std;

void print_board_state(const BitboardPosition& pos) {
    cout << "=== Board State Analysis ===" << endl;
    cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << endl;
    cout << "Castling rights: ";
    
    // Check castling rights by trying to access them
    cout << "White KS: " << (pos.castling_rights & 1 ? "Yes" : "No") << " ";
    cout << "White QS: " << (pos.castling_rights & 2 ? "Yes" : "No") << " ";
    cout << "Black KS: " << (pos.castling_rights & 4 ? "Yes" : "No") << " ";
    cout << "Black QS: " << (pos.castling_rights & 8 ? "Yes" : "No") << endl;
    
    cout << "En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank);
    }
    cout << endl;
    
    cout << "Halfmove clock: " << pos.halfmove_clock << endl;
    cout << "Fullmove number: " << pos.fullmove_number << endl;
}

int main() {
    cout << "=== Position Verification Test ===" << endl << endl;
    
    cout << "1. Starting position:" << endl;
    BitboardPosition start_pos;
    start_pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    print_board_state(start_pos);
    cout << endl;
    
    cout << "2. Position after a2a3 (what we're using):" << endl;
    BitboardPosition a2a3_pos;
    a2a3_pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    print_board_state(a2a3_pos);
    cout << endl;
    
    cout << "3. Manual verification: After 1.a3 from starting position" << endl;
    cout << "Expected position should be:" << endl;
    cout << "- Black to move (b)" << endl;
    cout << "- All castling rights intact (KQkq)" << endl;
    cout << "- No en passant target (-)" << endl;
    cout << "- Halfmove clock 0, Fullmove 1" << endl;
    cout << "- White pawn on a3, all other pieces in starting positions" << endl;
    cout << endl;
    
    cout << "4. Testing move generation counts:" << endl;
    BitboardMoveList moves;
    generate_all_moves(a2a3_pos, moves);
    
    int legal_moves = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move;
        simple_move.from_64 = move.from_64;
        simple_move.to_64 = move.to_64;
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        simple_move.promotion_type = move.promotion_type;
        
        if (a2a3_pos.is_legal_move(simple_move)) {
            legal_moves++;
        }
    }
    
    cout << "Legal moves from a2a3 position: " << legal_moves << endl;
    cout << "Expected: 20 (same as starting position since a3 doesn't block anything)" << endl;
    
    return 0;
}