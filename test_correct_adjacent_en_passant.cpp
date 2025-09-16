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
    cout << "=== Correct Adjacent File En Passant Test ===" << endl;
    
    // Test correct adjacent file scenario: a2a3 b7b5 c2c4 b5b4 c4c5 (should enable b4xc3)
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
    cout << "En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank) << " (square " << pos.ep_square_64 << ")";
    }
    cout << endl;
    
    // Check for en passant captures at this point (b5xc6)
    cout << "\n=== Checking for en passant captures (b5 can capture c6) ===" << endl;
    BitboardMoveList check_moves;
    generate_all_moves(pos, check_moves);
    
    int ep_count = 0;
    for (const auto& move : check_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            if (move.is_ep_capture) {
                ep_count++;
                cout << "  EN PASSANT CAPTURE: " << move_to_string_detailed(move) << endl;
            }
        }
    }
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
    
    // Try a different scenario: Instead of d2d4, let's try c4c5 to enable b4xc3 
    cout << "\n=== Testing alternative: c4c5 (invalid move, c4 is occupied) ===" << endl;
    cout << "c4 is occupied by a pawn that just moved there. Let's try a valid adjacent file scenario." << endl;
    
    cout << "\n=== Let's restart and try c7c5 d2d4 scenario ===" << endl;
    
    // Restart for a correct scenario
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // a2a3
    BitboardMoveList restart_moves1;
    generate_all_moves(pos, restart_moves1);
    for (const auto& move : restart_moves1.moves) {
        if (move_to_string_detailed(move) == "a2a3") {
            SimpleBitboardMove simple_move = convert_move(move);
            pos.make_move_with_undo(simple_move);
            break;
        }
    }
    
    // c7c5
    BitboardMoveList restart_moves2;
    generate_all_moves(pos, restart_moves2);
    for (const auto& move : restart_moves2.moves) {
        if (move_to_string_detailed(move) == "c7c5") {
            SimpleBitboardMove simple_move = convert_move(move);
            pos.make_move_with_undo(simple_move);
            break;
        }
    }
    
    // d2d4
    BitboardMoveList restart_moves3;
    generate_all_moves(pos, restart_moves3);
    for (const auto& move : restart_moves3.moves) {
        if (move_to_string_detailed(move) == "d2d4") {
            SimpleBitboardMove simple_move = convert_move(move);
            pos.make_move_with_undo(simple_move);
            break;
        }
    }
    cout << "En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank) << " (square " << pos.ep_square_64 << ")";
    }
    cout << endl;
    
    // Check for en passant captures (c5xd6 should be available - adjacent files c and d)
    cout << "\n=== Checking for en passant captures (c5 can capture d6) ===" << endl;
    BitboardMoveList final_moves;
    generate_all_moves(pos, final_moves);
    
    ep_count = 0;
    for (const auto& move : final_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            if (move.is_ep_capture) {
                ep_count++;
                cout << "  EN PASSANT CAPTURE: " << move_to_string_detailed(move) << endl;
                cout << "    From square " << move.from_64 << " to square " << move.to_64 << endl;
            }
        }
    }
    cout << "En passant captures found: " << ep_count << endl;
    
    if (ep_count > 0) {
        cout << "SUCCESS: En passant capture found!" << endl;
    } else {
        cout << "ERROR: Missing en passant capture c5xd6!" << endl;
    }
    
    return 0;
}