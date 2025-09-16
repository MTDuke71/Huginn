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

// Detailed perft divide for finding missing nodes
void detailed_divide(BitboardPosition& pos, int depth, const string& move_prefix) {
    if (depth <= 0) return;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    cout << "\n=== " << move_prefix << " - Depth " << depth << " divide ===" << endl;
    uint64_t total = 0;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            string move_str = move_to_string_detailed(move);
            
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t nodes = perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
            
            total += nodes;
            cout << move_str << " - " << nodes << endl;
            
            // If this is a 2-square pawn move by Black, drill down further
            if (depth >= 2 && move_str.length() == 4) {
                int from_rank = move_str[1] - '1';
                int to_rank = move_str[3] - '1';
                if (pos.side_to_move == Color::Black && from_rank == 6 && to_rank == 4) {
                    cout << "  -> This is a 2-square Black pawn move, drilling down..." << endl;
                    BitboardPosition::UndoInfo undo2 = pos.make_move_with_undo(simple_move);
                    detailed_divide(pos, 1, move_prefix + " " + move_str);
                    pos.unmake_move(simple_move, undo2);
                }
            }
        }
    }
    
    cout << "Total: " << total << endl;
}

int main() {
    cout << "=== Debug Next En Passant Bug: After a2a3 a7a5 c2c3 ===" << endl;
    
    // Set up the failing sequence: a2a3 a7a5 c2c3
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
    
    // Make c2c3
    BitboardMoveList moves2;
    generate_all_moves(pos, moves2);
    for (const auto& move : moves2.moves) {
        if (move_to_string_detailed(move) == "c2c3") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo2 = pos.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    cout << "Position after a2a3 a7a5 c2c3:" << endl;
    cout << "Side to move: " << (pos.side_to_move == Color::Black ? "Black" : "White") << endl;
    cout << "En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank);
    }
    cout << endl;
    
    // This should give us depth 3 perft = 9708, but expected is 9709 (-1 node)
    uint64_t current_perft = perft_legal(pos, 3);
    cout << "\nCurrent depth 3 perft: " << current_perft << endl;
    cout << "Expected depth 3 perft: 9709" << endl;
    cout << "Missing: " << (9709 - current_perft) << " nodes" << endl;
    
    // Do a detailed divide to find which Black moves are problematic
    detailed_divide(pos, 3, "a2a3 a7a5 c2c3");
    
    return 0;
}