#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <vector>
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
    cout << "=== Exact Re-trace of Fixed En Passant Bug ===" << endl;
    
    // Follow the exact sequence that we successfully fixed: a2a3 a7a5 a1a2 a5a4 b2b4
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    cout << "Starting position set up." << endl;
    
    // a2a3
    cout << "\n=== After a2a3 ===" << endl;
    BitboardMoveList moves1;
    generate_all_moves(pos, moves1);
    for (const auto& move : moves1.moves) {
        if (move_to_string_detailed(move) == "a2a3") {
            SimpleBitboardMove simple_move = convert_move(move);
            pos.make_move_with_undo(simple_move);
            break;
        }
    }
    cout << "Side to move: " << (pos.side_to_move == Color::Black ? "Black" : "White") << endl;
    
    // a7a5
    cout << "\n=== After a2a3 a7a5 ===" << endl;
    BitboardMoveList moves2;
    generate_all_moves(pos, moves2);
    for (const auto& move : moves2.moves) {
        if (move_to_string_detailed(move) == "a7a5") {
            SimpleBitboardMove simple_move = convert_move(move);
            pos.make_move_with_undo(simple_move);
            break;
        }
    }
    cout << "Side to move: " << (pos.side_to_move == Color::Black ? "Black" : "White") << endl;
    cout << "En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank) << " (square " << pos.ep_square_64 << ")";
    }
    cout << endl;
    
    // a1a2
    cout << "\n=== After a2a3 a7a5 a1a2 ===" << endl;
    BitboardMoveList moves3;
    generate_all_moves(pos, moves3);
    for (const auto& move : moves3.moves) {
        if (move_to_string_detailed(move) == "a1a2") {
            SimpleBitboardMove simple_move = convert_move(move);
            pos.make_move_with_undo(simple_move);
            break;
        }
    }
    cout << "Side to move: " << (pos.side_to_move == Color::Black ? "Black" : "White") << endl;
    cout << "En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank) << " (square " << pos.ep_square_64 << ")";
    }
    cout << endl;
    
    // a5a4
    cout << "\n=== After a2a3 a7a5 a1a2 a5a4 ===" << endl;
    BitboardMoveList moves4;
    generate_all_moves(pos, moves4);
    for (const auto& move : moves4.moves) {
        if (move_to_string_detailed(move) == "a5a4") {
            SimpleBitboardMove simple_move = convert_move(move);
            pos.make_move_with_undo(simple_move);
            break;
        }
    }
    cout << "Side to move: " << (pos.side_to_move == Color::Black ? "Black" : "White") << endl;
    cout << "En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank) << " (square " << pos.ep_square_64 << ")";
    }
    cout << endl;
    
    // b2b4
    cout << "\n=== After a2a3 a7a5 a1a2 a5a4 b2b4 ===" << endl;
    BitboardMoveList moves5;
    generate_all_moves(pos, moves5);
    for (const auto& move : moves5.moves) {
        if (move_to_string_detailed(move) == "b2b4") {
            SimpleBitboardMove simple_move = convert_move(move);
            pos.make_move_with_undo(simple_move);
            break;
        }
    }
    cout << "Side to move: " << (pos.side_to_move == Color::Black ? "Black" : "White") << endl;
    cout << "En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank) << " (square " << pos.ep_square_64 << ")";
    }
    cout << endl;
    
    // Now check for en passant captures
    cout << "\n=== Checking for en passant captures ===" << endl;
    BitboardMoveList final_moves;
    generate_all_moves(pos, final_moves);
    
    int ep_count = 0;
    int total_moves = 0;
    for (const auto& move : final_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            total_moves++;
            if (move.is_ep_capture) {
                ep_count++;
                cout << "  EN PASSANT CAPTURE: " << move_to_string_detailed(move) << endl;
                cout << "    From square " << move.from_64 << " to square " << move.to_64 << endl;
                cout << "    From " << square64_to_algebraic(move.from_64) << " to " << square64_to_algebraic(move.to_64) << endl;
            }
        }
    }
    
    cout << "Total legal moves: " << total_moves << endl;
    cout << "En passant captures found: " << ep_count << endl;
    
    // This should show a4xb3 en passant capture if the fix is working
    if (ep_count > 0) {
        cout << "SUCCESS: En passant capture found!" << endl;
    } else {
        cout << "ERROR: No en passant captures found" << endl;
    }
    
    return 0;
}