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
    cout << "=== Depth 2 Analysis: After a2a3 a7a5 a1a2 a5a4 (Reference Comparison) ===" << endl;
    
    // Reference data from user for depth 2 after a2a3 a7a5 a1a2 a5a4
    map<string, uint64_t> reference = {
        {"a2a1", 21}, {"b1c3", 21}, {"b2b3", 22}, {"b2b4", 22}, {"c2c3", 21}, {"c2c4", 21},
        {"d2d3", 21}, {"d2d4", 21}, {"e2e3", 21}, {"e2e4", 21}, {"f2f3", 21}, {"f2f4", 21},
        {"g1f3", 21}, {"g1h3", 21}, {"g2g3", 21}, {"g2g4", 21}, {"h2h3", 21}, {"h2h4", 21}
    };
    
    uint64_t reference_total = 380;
    
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
    
    cout << "Position after a2a3 a7a5 a1a2 a5a4:" << endl;
    cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << endl;
    cout << "En passant target: ";
    if (pos.ep_square_64 == -1) {
        cout << "None";
    } else {
        int file = pos.ep_square_64 % 8;
        int rank = pos.ep_square_64 / 8;
        cout << char('a' + file) << char('1' + rank);
    }
    cout << endl << endl;
    
    // Generate white moves and test each one (depth 2 = depth 1 after this white move)
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
            uint64_t nodes = perft_legal(pos, 1);  // depth 2 total = depth 1 after this move
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
    
    // Show missing moves in our results
    cout << "\nMoves in reference but missing from our results:" << endl;
    for (const auto& pair : reference) {
        string move_str = pair.first;
        auto our_it = our_results.find(move_str);
        if (our_it == our_results.end()) {
            cout << "  " << move_str << " (MISSING!)" << endl;
        }
    }
    
    // Show extra moves in our results
    cout << "\nExtra moves in our results not in reference:" << endl;
    for (const auto& pair : our_results) {
        string move_str = pair.first;
        auto ref_it = reference.find(move_str);
        if (ref_it == reference.end()) {
            cout << "  " << move_str << " (EXTRA!)" << endl;
        }
    }
    
    return 0;
}