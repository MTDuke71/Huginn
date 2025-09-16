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
    cout << "=== Depth 5 Analysis: After a2a3 ===" << endl;
    
    // Reference data from user for depth 5 after a2a3
    map<string, uint64_t> reference_data = {
        {"a7a5", 199263}, {"a7a6", 165948}, {"b7b5", 198797}, {"b7b6", 197301},
        {"b8a6", 182327}, {"b8c6", 215405}, {"c7c5", 219841}, {"c7c6", 203960},
        {"d7d5", 331473}, {"d7d6", 300892}, {"e7e5", 373372}, {"e7e6", 371069},
        {"f7f5", 181945}, {"f7f6", 163919}, {"g7g5", 196185}, {"g7g6", 199099},
        {"g8f6", 213990}, {"g8h6", 181930}, {"h7h5", 200588}, {"h7h6", 165963}
    };
    
    // Start from position after a2a3
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    cout << "Position after a2a3:" << endl;
    cout << "rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1" << endl;
    cout << "Expected total depth 5: 4463267" << endl;
    
    // Test our depth 5 result
    uint64_t our_total = perft_legal(pos, 5);
    cout << "Our total depth 5: " << our_total << endl;
    cout << "Difference: " << ((int64_t)our_total - 4463267) << endl << endl;
    
    // Now test individual moves from this position
    BitboardMoveList black_moves;
    generate_all_moves(pos, black_moves);
    
    cout << "Move     Our Result   Expected     Difference   Status" << endl;
    cout << "-------------------------------------------------------" << endl;
    
    uint64_t calculated_total = 0;
    for (const auto& black_move : black_moves.moves) {
        SimpleBitboardMove black_simple = convert_move(black_move);
        if (pos.is_legal_move(black_simple)) {
            BitboardPosition::UndoInfo black_undo = pos.make_move_with_undo(black_simple);
            
            uint64_t depth4_result = perft_legal(pos, 4);
            calculated_total += depth4_result;
            
            string black_move_str = move_to_string_detailed(black_move);
            uint64_t expected = 0;
            if (reference_data.find(black_move_str) != reference_data.end()) {
                expected = reference_data[black_move_str];
            }
            
            int64_t diff = (int64_t)depth4_result - (int64_t)expected;
            
            string status = (diff == 0) ? "✓ CORRECT" : 
                           (diff > 0) ? "✗ OVER" : "✗ UNDER";
            
            cout << left << setw(8) << black_move_str 
                 << " " << setw(11) << depth4_result
                 << " " << setw(11) << expected
                 << " " << setw(11) << diff
                 << " " << status << endl;
            
            pos.unmake_move(black_simple, black_undo);
        }
    }
    
    cout << "-------------------------------------------------------" << endl;
    cout << "TOTAL    " << setw(11) << calculated_total 
         << " " << setw(11) << 4463267
         << " " << setw(11) << ((int64_t)calculated_total - 4463267) << endl;
    
    cout << endl << "=== Analysis ===" << endl;
    if (calculated_total == our_total) {
        cout << "✅ Sum of individual moves matches direct perft calculation" << endl;
    } else {
        cout << "❌ Mismatch between sum (" << calculated_total << ") and direct perft (" << our_total << ")" << endl;
    }
    
    return 0;
}