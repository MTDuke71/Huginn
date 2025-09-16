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
    cout << "=== Find Another En Passant Bug: b7b5 c2c4 Scenario ===" << endl;
    
    // Try a different sequence: a2a3 b7b5 c2c4 b5b4 d2d4 (should enable b4xc3)
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
    
    // b7b5
    cout << "\n=== After a2a3 b7b5 ===" << endl;
    BitboardMoveList moves2;
    generate_all_moves(pos, moves2);
    for (const auto& move : moves2.moves) {
        if (move_to_string_detailed(move) == "b7b5") {
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
    
    // c2c4
    cout << "\n=== After a2a3 b7b5 c2c4 ===" << endl;
    BitboardMoveList moves3;
    generate_all_moves(pos, moves3);
    for (const auto& move : moves3.moves) {
        if (move_to_string_detailed(move) == "c2c4") {
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
    
    // Look for en passant captures available here
    cout << "\n=== Checking for en passant captures (b5xc6 should be available) ===" << endl;
    BitboardMoveList check_moves;
    generate_all_moves(pos, check_moves);
    
    int ep_count = 0;
    int total_moves = 0;
    for (const auto& move : check_moves.moves) {
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
    
    // b5b4
    cout << "\n=== After a2a3 b7b5 c2c4 b5b4 ===" << endl;
    BitboardMoveList moves4;
    generate_all_moves(pos, moves4);
    for (const auto& move : moves4.moves) {
        if (move_to_string_detailed(move) == "b5b4") {
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
    
    // d2d4
    cout << "\n=== After a2a3 b7b5 c2c4 b5b4 d2d4 ===" << endl;
    BitboardMoveList moves5;
    generate_all_moves(pos, moves5);
    for (const auto& move : moves5.moves) {
        if (move_to_string_detailed(move) == "d2d4") {
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
    cout << "\n=== Checking for en passant captures (b4xd3 should be available) ===" << endl;
    BitboardMoveList final_moves;
    generate_all_moves(pos, final_moves);
    
    ep_count = 0;
    total_moves = 0;
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
    
    if (ep_count > 0) {
        cout << "SUCCESS: En passant capture found!" << endl;
    } else {
        cout << "ERROR: Missing en passant capture! This is likely another bug." << endl;
    }
    
    return 0;
}