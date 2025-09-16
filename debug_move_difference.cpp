#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <set>
#include <string>

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

set<string> get_legal_moves(BitboardPosition& pos) {
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    set<string> legal_moves;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            legal_moves.insert(move_to_string_detailed(move));
        }
    }
    return legal_moves;
}

int main() {
    cout << "=== Comparing move differences after a2a3 a7a5 vs a2a3 a7a6 ===" << endl;
    
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
    
    // Make c2c3
    BitboardMoveList moves2;
    generate_all_moves(pos1, moves2);
    for (const auto& move : moves2.moves) {
        if (move_to_string_detailed(move) == "c2c3") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos1.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo2 = pos1.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    cout << "After a2a3 a7a5 c2c3:" << endl;
    cout << "En passant target: ";
    if (pos1.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos1.ep_square_64 % 8;
        int rank = pos1.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank);
    }
    cout << endl;
    
    set<string> moves_a7a5 = get_legal_moves(pos1);
    cout << "Black moves: " << moves_a7a5.size() << endl;
    
    // === Test with a7a6 (no en passant target) ===
    BitboardPosition pos2;
    pos2.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    // Make a7a6
    BitboardMoveList moves3;
    generate_all_moves(pos2, moves3);
    for (const auto& move : moves3.moves) {
        if (move_to_string_detailed(move) == "a7a6") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos2.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo3 = pos2.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    // Make c2c3
    BitboardMoveList moves4;
    generate_all_moves(pos2, moves4);
    for (const auto& move : moves4.moves) {
        if (move_to_string_detailed(move) == "c2c3") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos2.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo4 = pos2.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    cout << "\nAfter a2a3 a7a6 c2c3:" << endl;
    cout << "En passant target: ";
    if (pos2.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos2.ep_square_64 % 8;
        int rank = pos2.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank);
    }
    cout << endl;
    
    set<string> moves_a7a6 = get_legal_moves(pos2);
    cout << "Black moves: " << moves_a7a6.size() << endl;
    
    // Find differences
    cout << "\nMoves in a7a6 scenario but NOT in a7a5 scenario:" << endl;
    for (const string& move : moves_a7a6) {
        if (moves_a7a5.find(move) == moves_a7a5.end()) {
            cout << "  " << move << endl;
        }
    }
    
    cout << "\nMoves in a7a5 scenario but NOT in a7a6 scenario:" << endl;
    for (const string& move : moves_a7a5) {
        if (moves_a7a6.find(move) == moves_a7a6.end()) {
            cout << "  " << move << endl;
        }
    }
    
    return 0;
}