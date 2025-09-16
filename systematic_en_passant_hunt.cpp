#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
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

// Test a specific sequence and look for en passant issues
pair<uint64_t, uint64_t> test_sequence(const vector<string>& moves, int target_depth) {
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Play the sequence
    for (const string& move_str : moves) {
        BitboardMoveList available_moves;
        generate_all_moves(pos, available_moves);
        
        bool found = false;
        for (const auto& move : available_moves.moves) {
            if (move_to_string_detailed(move) == move_str) {
                SimpleBitboardMove simple_move = convert_move(move);
                if (pos.is_legal_move(simple_move)) {
                    pos.make_move_with_undo(simple_move);
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            cout << "ERROR: Could not find move " << move_str << endl;
            return {0, 0};
        }
    }
    
    // Count en passant captures available
    BitboardMoveList final_moves;
    generate_all_moves(pos, final_moves);
    uint64_t ep_captures = 0;
    for (const auto& move : final_moves.moves) {
        if (move.is_ep_capture) {
            ep_captures++;
            cout << "  EN PASSANT AVAILABLE: " << move_to_string_detailed(move) << endl;
        }
    }
    
    uint64_t perft_result = perft_legal(pos, target_depth);
    return {perft_result, ep_captures};
}

int main() {
    cout << "=== Systematic En Passant Bug Hunt ===" << endl;
    
    // Test known failing sequences from the reference data
    vector<vector<string>> failing_sequences = {
        {"a2a3", "a7a5", "b2b4"},     // This was the originally fixed sequence
        {"a2a3", "a7a5", "c2c3"},    // Expected 9709, was getting 9708
        {"a2a3", "a7a5", "d2d3"},    // Expected 12688, was getting 12687
        {"a2a3", "b7b5", "c2c4"},    // Try a different 2-square pawn scenario
        {"a2a3", "c7c5", "b2b4"},    // Another potential en passant scenario
        {"a2a3", "d7d5", "c2c4"},    // Yet another scenario
        {"a2a3", "e7e5", "d2d4"},    // Test e-file en passant
        {"a2a3", "f7f5", "e2e4"},    // Test f-file en passant
        {"a2a3", "g7g5", "f2f4"},    // Test g-file en passant
        {"a2a3", "h7h5", "g2g4"},    // Test h-file en passant
    };
    
    for (const auto& sequence : failing_sequences) {
        cout << "\n=== Testing sequence: ";
        for (size_t i = 0; i < sequence.size(); ++i) {
            cout << sequence[i];
            if (i < sequence.size() - 1) cout << " ";
        }
        cout << " ===" << endl;
        
        auto [perft_result, ep_count] = test_sequence(sequence, 3);
        cout << "Perft depth 3: " << perft_result << ", En passant captures available: " << ep_count << endl;
    }
    
    cout << "\n=== Testing individual 2-square pawn moves for en passant setup ===" << endl;
    
    // Test each Black 2-square pawn move after a2a3 to see if en passant is correctly set up
    vector<string> black_pawn_moves = {"a7a5", "b7b5", "c7c5", "d7d5", "e7e5", "f7f5", "g7g5", "h7h5"};
    
    for (const string& pawn_move : black_pawn_moves) {
        cout << "\n--- After a2a3 " << pawn_move << " ---" << endl;
        
        BitboardPosition pos;
        pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/P7/1PPPPPPP/RNBQKBNR b KQkq - 0 1");
        
        // Make the pawn move
        BitboardMoveList moves;
        generate_all_moves(pos, moves);
        for (const auto& move : moves.moves) {
            if (move_to_string_detailed(move) == pawn_move) {
                SimpleBitboardMove simple_move = convert_move(move);
                if (pos.is_legal_move(simple_move)) {
                    pos.make_move_with_undo(simple_move);
                    break;
                }
            }
        }
        
        cout << "En passant square: ";
        if (pos.ep_square_64 == -1) {
            cout << "None";
        } else {
            int file = pos.ep_square_64 % 8;
            int rank = pos.ep_square_64 / 8;
            cout << char('a' + file) << char('1' + rank);
        }
        cout << endl;
        
        // Now check each White response that could potentially capture en passant
        BitboardMoveList white_moves;
        generate_all_moves(pos, white_moves);
        
        bool found_any_ep = false;
        for (const auto& move : white_moves.moves) {
            if (move.is_ep_capture) {
                found_any_ep = true;
                cout << "  Potential en passant: " << move_to_string_detailed(move) << endl;
            }
        }
        
        if (!found_any_ep) {
            cout << "  No en passant captures available" << endl;
        }
    }
    
    return 0;
}