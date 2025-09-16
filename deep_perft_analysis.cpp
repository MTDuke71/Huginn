#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <map>
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

// Perft function with detailed divide
struct PerftResult {
    map<string, uint64_t> move_nodes;
    uint64_t total_nodes;
};

PerftResult perft_divide(BitboardPosition& pos, int depth) {
    PerftResult result;
    result.total_nodes = 0;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            string move_str = square64_to_algebraic(move.from_64) + square64_to_algebraic(move.to_64);
            if (move.is_promotion) {
                switch (move.promotion_type) {
                    case PieceType::Queen: move_str += "q"; break;
                    case PieceType::Rook: move_str += "r"; break;
                    case PieceType::Bishop: move_str += "b"; break;
                    case PieceType::Knight: move_str += "n"; break;
                    default: break;
                }
            }
            
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t nodes = (depth <= 1) ? 1 : perft_divide(pos, depth - 1).total_nodes;
            pos.unmake_move(simple_move, undo);
            
            result.move_nodes[move_str] = nodes;
            result.total_nodes += nodes;
        }
    }
    
    return result;
}

// Convert 64-square to algebraic notation
string square64_to_algebraic(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return string(1, 'a' + file) + string(1, '1' + rank);
}

void deep_compare_positions(const string& move1, const string& move2, BitboardPosition& base_pos) {
    cout << "\n=== DEEP COMPARISON: " << move1 << " vs " << move2 << " ===" << endl;
    
    // Test move1 (working)
    BitboardPosition pos1 = base_pos;
    BitboardMoveList moves1;
    generate_all_moves(pos1, moves1);
    for (const auto& move : moves1.moves) {
        string move_str = square64_to_algebraic(move.from_64) + square64_to_algebraic(move.to_64);
        if (move_str == move1) {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos1.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo1 = pos1.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    // Test move2 (failing)
    BitboardPosition pos2 = base_pos;
    BitboardMoveList moves2;
    generate_all_moves(pos2, moves2);
    for (const auto& move : moves2.moves) {
        string move_str = square64_to_algebraic(move.from_64) + square64_to_algebraic(move.to_64);
        if (move_str == move2) {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos2.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo2 = pos2.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    // Compare depth 1 and depth 2 perft divides
    cout << "Depth 1 divide after " << move1 << ":" << endl;
    PerftResult result1_d1 = perft_divide(pos1, 1);
    cout << "Total: " << result1_d1.total_nodes << endl;
    
    cout << "\nDepth 1 divide after " << move2 << ":" << endl;
    PerftResult result2_d1 = perft_divide(pos2, 1);
    cout << "Total: " << result2_d1.total_nodes << endl;
    
    if (result1_d1.total_nodes != result2_d1.total_nodes) {
        cout << "DIFFERENCE AT DEPTH 1!" << endl;
        return;
    }
    
    cout << "\nDepth 2 divide after " << move1 << ":" << endl;
    PerftResult result1_d2 = perft_divide(pos1, 2);
    cout << "Total: " << result1_d2.total_nodes << endl;
    
    cout << "\nDepth 2 divide after " << move2 << ":" << endl;
    PerftResult result2_d2 = perft_divide(pos2, 2);
    cout << "Total: " << result2_d2.total_nodes << endl;
    
    if (result1_d2.total_nodes != result2_d2.total_nodes) {
        cout << "\nDIFFERENCE AT DEPTH 2! Analyzing specific moves..." << endl;
        
        for (const auto& pair : result1_d2.move_nodes) {
            string black_move = pair.first;
            uint64_t nodes1 = pair.second;
            auto it2 = result2_d2.move_nodes.find(black_move);
            if (it2 != result2_d2.move_nodes.end()) {
                uint64_t nodes2 = it2->second;
                if (nodes1 != nodes2) {
                    cout << "  " << black_move << ": " << move1 << " gives " << nodes1 
                         << ", " << move2 << " gives " << nodes2 
                         << " (diff: " << (int64_t)nodes1 - (int64_t)nodes2 << ")" << endl;
                }
            } else {
                cout << "  " << black_move << ": only available after " << move1 << endl;
            }
        }
        
        for (const auto& pair : result2_d2.move_nodes) {
            string black_move = pair.first;
            auto it1 = result1_d2.move_nodes.find(black_move);
            if (it1 == result1_d2.move_nodes.end()) {
                cout << "  " << black_move << ": only available after " << move2 << endl;
            }
        }
    }
}

int main() {
    cout << "=== Deep Perft Tree Analysis: After a2a3 a7a5 ===" << endl;
    
    // Set up position after a2a3 a7a5
    BitboardPosition base_pos;
    base_pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    // Make a7a5
    BitboardMoveList moves;
    generate_all_moves(base_pos, moves);
    for (const auto& move : moves.moves) {
        string move_str = square64_to_algebraic(move.from_64) + square64_to_algebraic(move.to_64);
        if (move_str == "a7a5") {
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
    cout << endl;
    
    // Compare working move (b2b3) vs failing move (c2c3)
    deep_compare_positions("b2b3", "c2c3", base_pos);
    
    return 0;
}