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
    cout << "=== Depth 4 Analysis: After a2a3 a7a6 ===" << endl;
    
    // Reference data from user for depth 4 after a2a3 a7a6
    map<string, uint64_t> reference_data = {
        {"a1a2", 7754}, {"a3a4", 8551}, {"b1c3", 9368}, {"b2b3", 7754}, {"b2b4", 7759},
        {"c2c3", 8091}, {"c2c4", 8525}, {"d2d3", 10555}, {"d2d4", 10993}, {"e2e3", 11695},
        {"e2e4", 11720}, {"f2f3", 7346}, {"f2f4", 7780}, {"g1f3", 8528}, {"g1h3", 7734},
        {"g2g3", 8160}, {"g2g4", 8144}, {"h2h3", 7346}, {"h2h4", 8145}
    };
    
    // Start from position after a2a3 a7a6
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    // Make a7a6 move
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    bool found_a7a6 = false;
    for (const auto& move : moves.moves) {
        string move_str = move_to_string_detailed(move);
        if (move_str == "a7a6") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
                found_a7a6 = true;
                
                cout << "Position after a2a3 a7a6:" << endl;
                cout << "Expected total depth 4: 165948" << endl;
                
                // Test our depth 4 result
                uint64_t our_total = perft_legal(pos, 4);
                cout << "Our total depth 4: " << our_total << endl;
                cout << "Difference: " << ((int64_t)our_total - 165948) << endl << endl;
                
                // Now test individual moves from this position
                BitboardMoveList white_moves;
                generate_all_moves(pos, white_moves);
                
                cout << "Move     Our Result   Expected     Difference   Status" << endl;
                cout << "-------------------------------------------------------" << endl;
                
                uint64_t calculated_total = 0;
                for (const auto& white_move : white_moves.moves) {
                    SimpleBitboardMove white_simple = convert_move(white_move);
                    if (pos.is_legal_move(white_simple)) {
                        BitboardPosition::UndoInfo white_undo = pos.make_move_with_undo(white_simple);
                        
                        uint64_t depth3_result = perft_legal(pos, 3);
                        calculated_total += depth3_result;
                        
                        string white_move_str = move_to_string_detailed(white_move);
                        uint64_t expected = reference_data[white_move_str];
                        int64_t diff = (int64_t)depth3_result - (int64_t)expected;
                        
                        string status = (diff == 0) ? "✓ CORRECT" : 
                                       (diff > 0) ? "✗ OVER" : "✗ UNDER";
                        
                        cout << left << setw(8) << white_move_str 
                             << " " << setw(11) << depth3_result
                             << " " << setw(11) << expected
                             << " " << setw(11) << diff
                             << " " << status << endl;
                        
                        pos.unmake_move(white_simple, white_undo);
                    }
                }
                
                cout << "-------------------------------------------------------" << endl;
                cout << "TOTAL    " << setw(11) << calculated_total 
                     << " " << setw(11) << 165948
                     << " " << setw(11) << ((int64_t)calculated_total - 165948) << endl;
                
                pos.unmake_move(simple_move, undo);
                break;
            }
        }
    }
    
    if (!found_a7a6) {
        cout << "ERROR: Could not find or make move a7a6" << endl;
    }
    
    return 0;
}