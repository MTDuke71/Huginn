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

int main() {
    cout << "=== Verification: Missing En Passant Capture a4xb3 ===" << endl;
    
    // Set up position after the complete sequence
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    // Make a7a5
    BitboardMoveList moves1;
    generate_all_moves(pos, moves1);
    for (const auto& move : moves1.moves) {
        if (move_to_string_detailed(move) == "a7a5") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo1 = pos.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    // Make a1a2
    BitboardMoveList moves2;
    generate_all_moves(pos, moves2);
    for (const auto& move : moves2.moves) {
        if (move_to_string_detailed(move) == "a1a2") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo2 = pos.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    // Make a5a4
    BitboardMoveList moves3;
    generate_all_moves(pos, moves3);
    for (const auto& move : moves3.moves) {
        if (move_to_string_detailed(move) == "a5a4") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo3 = pos.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    // Make b2b4
    BitboardMoveList moves4;
    generate_all_moves(pos, moves4);
    for (const auto& move : moves4.moves) {
        if (move_to_string_detailed(move) == "b2b4") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo4 = pos.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    cout << "Position after a2a3 a7a5 a1a2 a5a4 b2b4:" << endl;
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
    
    // Show position details
    cout << "\nPosition details:" << endl;
    cout << "Black pawn on a4: square " << (3*8 + 0) << " = " << (pos.piece_at(3*8 + 0) == Piece::BlackPawn ? "YES" : "NO") << endl;
    cout << "White pawn on b4: square " << (3*8 + 1) << " = " << (pos.piece_at(3*8 + 1) == Piece::WhitePawn ? "YES" : "NO") << endl;
    
    // Generate all black moves
    BitboardMoveList black_moves;
    generate_all_moves(pos, black_moves);
    
    cout << "\nBlack moves available: " << endl;
    set<string> legal_moves;
    int move_count = 0;
    
    for (const auto& move : black_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            string move_str = move_to_string_detailed(move);
            legal_moves.insert(move_str);
            
            cout << move_count + 1 << ". " << move_str;
            if (simple_move.is_ep_capture) {
                cout << " (EN PASSANT)";
            }
            if (simple_move.is_capture) {
                cout << " (CAPTURE)";
            }
            cout << endl;
            
            move_count++;
        }
    }
    
    cout << "\nTotal legal moves: " << move_count << endl;
    cout << "Expected: 22 moves" << endl;
    
    // Check specifically for a4b3
    if (legal_moves.find("a4b3") != legal_moves.end()) {
        cout << "✓ a4b3 en passant capture is PRESENT" << endl;
    } else {
        cout << "✗ a4b3 en passant capture is MISSING!" << endl;
        
        // Debug en passant generation
        cout << "\nDEBUGGING EN PASSANT GENERATION:" << endl;
        cout << "En passant target: " << pos.ep_square_64 << endl;
        if (pos.ep_square_64 != -1) {
            cout << "Expected black pawn on a4 to capture to b3" << endl;
            cout << "a4 square index: " << (3*8 + 0) << endl;
            cout << "b3 square index: " << (2*8 + 1) << endl;
            cout << "En passant target should be: " << (2*8 + 1) << endl;
        }
    }
    
    return 0;
}