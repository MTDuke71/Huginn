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
    cout << "=== Depth 4 Analysis: After a2a3 a7a5 (Reference Comparison) ===" << endl;
    
    // Reference data from user
    map<string, uint64_t> reference = {
        {"a1a2", 9312}, {"a3a4", 9062}, {"b1c3", 11218}, {"b2b3", 9349}, {"b2b4", 10545},
        {"c2c3", 9709}, {"c2c4", 10227}, {"d2d3", 12688}, {"d2d4", 13208}, {"e2e3", 14016},
        {"e2e4", 14041}, {"f2f3", 8821}, {"f2f4", 9338}, {"g1f3", 10248}, {"g1h3", 9290},
        {"g2g3", 9801}, {"g2g4", 9785}, {"h2h3", 8821}, {"h2h4", 9784}
    };
    
    uint64_t reference_total = 199263;
    
    // Set up position after a2a3
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    // Make a7a5
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    for (const auto& move : moves.moves) {
        if (move_to_string_detailed(move) == "a7a5") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
                break;
            }
        }
    }
    
    cout << "Position after a2a3 a7a5:" << endl;
    cout << "En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank);
    }
    cout << endl << endl;
    
    // Generate white moves and test each one
    BitboardMoveList white_moves;
    generate_all_moves(pos, white_moves);
    
    map<string, uint64_t> our_results;
    uint64_t our_total = 0;
    
    cout << "Move     Our Result   Expected     Difference   Status" << endl;
    cout << "-------------------------------------------------------" << endl;
    
    for (const auto& move : white_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            string move_str = move_to_string_detailed(move);
            
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t nodes = perft_legal(pos, 3);  // depth 4 total = depth 3 after this move
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
            }
        }
    }
    
    cout << "-------------------------------------------------------" << endl;
    cout << "TOTAL    " << setw(12) << our_total 
         << setw(12) << reference_total 
         << setw(12) << ((int64_t)our_total - (int64_t)reference_total) << endl;
    
    cout << "\n=== Analysis ===" << endl;
    if (our_total == reference_total) {
        cout << "✓ Total matches reference - no systematic error" << endl;
    } else {
        cout << "✗ Total differs by " << ((int64_t)our_total - (int64_t)reference_total) << " nodes" << endl;
    }
    
    // Show moves with errors
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
    
    return 0;
}