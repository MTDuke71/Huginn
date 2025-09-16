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

void test_after_white_move(BitboardPosition& base_pos, const string& white_move_str) {
    BitboardPosition pos = base_pos;
    
    // Make the white move
    BitboardMoveList white_moves;
    generate_all_moves(pos, white_moves);
    
    bool found_move = false;
    for (const auto& move : white_moves.moves) {
        if (move_to_string_detailed(move) == white_move_str) {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
                found_move = true;
                break;
            }
        }
    }
    
    if (!found_move) {
        cout << "Could not find move: " << white_move_str << endl;
        return;
    }
    
    // Get black's legal moves
    set<string> black_moves = get_legal_moves(pos);
    
    cout << "After " << white_move_str << ":" << endl;
    cout << "  En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank);
    }
    cout << endl;
    cout << "  Black moves: " << black_moves.size() << endl;
    
    // Print a few sample moves
    cout << "  Sample moves: ";
    int count = 0;
    for (const string& move : black_moves) {
        if (count >= 5) break;
        cout << move << " ";
        count++;
    }
    cout << "..." << endl;
}

int main() {
    cout << "=== Identifying Missing Black Move: After a2a3 a7a5 ===" << endl;
    
    // Set up position after a2a3 a7a5
    BitboardPosition base_pos;
    base_pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    // Make a7a5
    BitboardMoveList moves;
    generate_all_moves(base_pos, moves);
    for (const auto& move : moves.moves) {
        if (move_to_string_detailed(move) == "a7a5") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (base_pos.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo = base_pos.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    cout << "Base position after a2a3 a7a5:" << endl;
    cout << "En passant target: ";
    if (base_pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = base_pos.ep_square_64 % 8;
        int rank = base_pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank);
    }
    cout << endl << endl;
    
    // Test the two moves that work correctly
    cout << "=== WORKING MOVES (correct node count) ===" << endl;
    test_after_white_move(base_pos, "b2b3");
    test_after_white_move(base_pos, "a3a4");
    
    cout << "\n=== FAILING MOVES (missing 1 node) ===" << endl;
    test_after_white_move(base_pos, "c2c3");
    test_after_white_move(base_pos, "d2d3");
    test_after_white_move(base_pos, "b1c3");
    
    // Now let's compare the move sets directly
    cout << "\n=== DETAILED COMPARISON: b2b3 vs c2c3 ===" << endl;
    
    BitboardPosition pos1 = base_pos;
    BitboardPosition pos2 = base_pos;
    
    // Make b2b3 (working)
    BitboardMoveList moves1;
    generate_all_moves(pos1, moves1);
    for (const auto& move : moves1.moves) {
        if (move_to_string_detailed(move) == "b2b3") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos1.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo1 = pos1.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    // Make c2c3 (failing)
    BitboardMoveList moves2;
    generate_all_moves(pos2, moves2);
    for (const auto& move : moves2.moves) {
        if (move_to_string_detailed(move) == "c2c3") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos2.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo2 = pos2.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    set<string> moves_after_b2b3 = get_legal_moves(pos1);
    set<string> moves_after_c2c3 = get_legal_moves(pos2);
    
    cout << "After b2b3: " << moves_after_b2b3.size() << " moves" << endl;
    cout << "After c2c3: " << moves_after_c2c3.size() << " moves" << endl;
    
    cout << "\nMoves in b2b3 but NOT in c2c3:" << endl;
    for (const string& move : moves_after_b2b3) {
        if (moves_after_c2c3.find(move) == moves_after_c2c3.end()) {
            cout << "  " << move << " (MISSING from c2c3)" << endl;
        }
    }
    
    cout << "\nMoves in c2c3 but NOT in b2b3:" << endl;
    for (const string& move : moves_after_c2c3) {
        if (moves_after_b2b3.find(move) == moves_after_b2b3.end()) {
            cout << "  " << move << " (EXTRA in c2c3)" << endl;
        }
    }
    
    return 0;
}