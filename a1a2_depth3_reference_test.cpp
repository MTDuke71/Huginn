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
    cout << "=== Depth 3 Analysis: After a2a3 a7a5 a1a2 (Reference Comparison) ===" << endl;
    
    // Reference data from user for depth 3 after a2a3 a7a5 a1a2
    map<string, uint64_t> reference = {
        {"a5a4", 380}, {"a8a6", 513}, {"a8a7", 399}, {"b7b5", 419}, {"b7b6", 418},
        {"b8a6", 418}, {"b8c6", 437}, {"c7c5", 419}, {"c7c6", 399}, {"d7d5", 551},
        {"d7d6", 531}, {"e7e5", 589}, {"e7e6", 588}, {"f7f5", 400}, {"f7f6", 380},
        {"g7g5", 419}, {"g7g6", 418}, {"g8f6", 437}, {"g8h6", 399}, {"h7h5", 418}, {"h7h6", 380}
    };
    
    uint64_t reference_total = 9312;
    
    // Set up position after a2a3
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
    
    cout << "Position after a2a3 a7a5 a1a2:" << endl;
    cout << "Side to move: " << (pos.side_to_move == Color::Black ? "Black" : "White") << endl;
    cout << "En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank);
    }
    cout << endl << endl;
    
    // Generate black moves and test each one (depth 3 = depth 2 after this black move)
    BitboardMoveList black_moves;
    generate_all_moves(pos, black_moves);
    
    map<string, uint64_t> our_results;
    uint64_t our_total = 0;
    
    cout << "Move     Our Result   Expected     Difference   Status" << endl;
    cout << "-------------------------------------------------------" << endl;
    
    for (const auto& move : black_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            string move_str = move_to_string_detailed(move);
            
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t nodes = perft_legal(pos, 2);  // depth 3 total = depth 2 after this move
            pos.unmake_move(simple_move, undo);
            
            our_results[move_str] = nodes;
            our_total += nodes;
            
            auto ref_it = reference.find(move_str);
            if (ref_it != reference.end()) {
                uint64_t expected = ref_it->second;
                int64_t diff = (int64_t)nodes - (int64_t)expected;
                
                cout << left << setw(9) << move_str 
                     << setw(12) << nodes 
                     << setw(12) << expected 
                     << setw(12) << diff
                     << (diff == 0 ? "✓ CORRECT" : "✗ ERROR") << endl;
            } else {
                cout << left << setw(9) << move_str 
                     << setw(12) << nodes 
                     << setw(12) << "N/A"
                     << setw(12) << "N/A"
                     << "NOT IN REF" << endl;
            }
        }
    }
    
    cout << "-------------------------------------------------------" << endl;
    cout << "TOTAL    " << setw(12) << our_total 
         << setw(12) << reference_total 
         << setw(12) << ((int64_t)our_total - (int64_t)reference_total) << endl;
    
    cout << "\n=== Analysis ===" << endl;
    if (our_total == reference_total) {
        cout << "✓ Total matches reference" << endl;
    } else {
        cout << "✗ Total differs by " << ((int64_t)our_total - (int64_t)reference_total) << " nodes" << endl;
        
        cout << "\nMoves with errors:" << endl;
        for (const auto& pair : reference) {
            string move_str = pair.first;
            uint64_t expected = pair.second;
            auto our_it = our_results.find(move_str);
            if (our_it != our_results.end()) {
                uint64_t our_result = our_it->second;
                if (our_result != expected) {
                    cout << "  " << move_str << ": got " << our_result << ", expected " << expected 
                         << " (diff: " << ((int64_t)our_result - (int64_t)expected) << ")" << endl;
                }
            }
        }
    }
    
    return 0;
}