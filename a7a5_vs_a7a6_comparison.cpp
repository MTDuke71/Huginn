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
    cout << "=== Comparison: a7a5 vs a7a6 (2-square vs 1-square pawn) ===" << endl;
    
    // Reference data for depth 4 after a2a3 a7a5 (2-square pawn move)
    map<string, uint64_t> a7a5_reference = {
        {"a1a2", 9312}, {"a3a4", 9062}, {"b1c3", 11218}, {"b2b3", 9349}, {"b2b4", 10545},
        {"c2c3", 9709}, {"c2c4", 10227}, {"d2d3", 12688}, {"d2d4", 13208}, {"e2e3", 14016},
        {"e2e4", 14041}, {"f2f3", 8821}, {"f2f4", 9338}, {"g1f3", 10248}, {"g1h3", 9290},
        {"g2g3", 9801}, {"g2g4", 9785}, {"h2h3", 8821}, {"h2h4", 9784}
    };
    
    // Reference data for depth 4 after a2a3 a7a6 (1-square pawn move)
    map<string, uint64_t> a7a6_reference = {
        {"a1a2", 7754}, {"a3a4", 8551}, {"b1c3", 9368}, {"b2b3", 7754}, {"b2b4", 7759},
        {"c2c3", 8091}, {"c2c4", 8525}, {"d2d3", 10555}, {"d2d4", 10993}, {"e2e3", 11695},
        {"e2e4", 11720}, {"f2f3", 7346}, {"f2f4", 7780}, {"g1f3", 8528}, {"g1h3", 7734},
        {"g2g3", 8160}, {"g2g4", 8144}, {"h2h3", 7346}, {"h2h4", 8145}
    };
    
    cout << "1. Testing a2a3 a7a5 (2-square pawn move, sets en passant target):" << endl;
    
    // Start from position after a2a3
    BitboardPosition pos_a7a5;
    pos_a7a5.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
    
    // Make a7a5 move (2-square pawn move)
    BitboardMoveList moves;
    generate_all_moves(pos_a7a5, moves);
    
    bool found_a7a5 = false;
    for (const auto& move : moves.moves) {
        string move_str = move_to_string_detailed(move);
        if (move_str == "a7a5") {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos_a7a5.is_legal_move(simple_move)) {
                BitboardPosition::UndoInfo undo = pos_a7a5.make_move_with_undo(simple_move);
                found_a7a5 = true;
                
                cout << "Position after a2a3 a7a5 - En passant target should be a6" << endl;
                cout << "En passant target: ";
                if (pos_a7a5.ep_square_64 == -1) {
                    cout << "None (BUG!)";
                } else {
                    int file = pos_a7a5.ep_square_64 % 8;
                    int rank = pos_a7a5.ep_square_64 / 8;
                    cout << char('a' + file) << char('1' + rank);
                }
                cout << endl;
                
                // Test our depth 4 result
                uint64_t our_total = perft_legal(pos_a7a5, 4);
                cout << "Expected total: 199263" << endl;
                cout << "Our total: " << our_total << endl;
                cout << "Difference: " << ((int64_t)our_total - 199263) << endl << endl;
                
                // Test individual moves
                BitboardMoveList white_moves;
                generate_all_moves(pos_a7a5, white_moves);
                
                cout << "Move     Our Result   Expected     A7A6 Ref     Extra vs A7A6" << endl;
                cout << "----------------------------------------------------------------" << endl;
                
                for (const auto& white_move : white_moves.moves) {
                    SimpleBitboardMove white_simple = convert_move(white_move);
                    if (pos_a7a5.is_legal_move(white_simple)) {
                        BitboardPosition::UndoInfo white_undo = pos_a7a5.make_move_with_undo(white_simple);
                        
                        uint64_t depth3_result = perft_legal(pos_a7a5, 3);
                        
                        string white_move_str = move_to_string_detailed(white_move);
                        uint64_t expected_a7a5 = a7a5_reference[white_move_str];
                        uint64_t expected_a7a6 = a7a6_reference[white_move_str];
                        
                        int64_t diff_from_expected = (int64_t)depth3_result - (int64_t)expected_a7a5;
                        int64_t extra_vs_a7a6 = (int64_t)expected_a7a5 - (int64_t)expected_a7a6;
                        
                        cout << left << setw(8) << white_move_str 
                             << " " << setw(11) << depth3_result
                             << " " << setw(11) << expected_a7a5
                             << " " << setw(11) << expected_a7a6
                             << " " << setw(11) << extra_vs_a7a6;
                        
                        if (diff_from_expected != 0) {
                            cout << " ❌ (" << diff_from_expected << ")";
                        } else if (extra_vs_a7a6 > 0) {
                            cout << " ✅ EP";
                        }
                        cout << endl;
                        
                        pos_a7a5.unmake_move(white_simple, white_undo);
                    }
                }
                
                pos_a7a5.unmake_move(simple_move, undo);
                break;
            }
        }
    }
    
    if (!found_a7a5) {
        cout << "ERROR: Could not find or make move a7a5" << endl;
    }
    
    return 0;
}