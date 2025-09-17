/**
 * @file debug_kiwipete_moves.cpp
 * @brief Debug Kiwipete depth 2 move-by-move to find missing moves
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

string square_to_string(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return string(1, 'a' + file) + string(1, '1' + rank);
}

string move_to_string(const BitboardMoveList::BitboardMove& move) {
    string result = square_to_string(move.from_64) + square_to_string(move.to_64);
    if (move.promotion_piece != PieceType::None) {
        switch (move.promotion_piece) {
            case PieceType::Queen: result += "q"; break;
            case PieceType::Rook: result += "r"; break;
            case PieceType::Bishop: result += "b"; break;
            case PieceType::Knight: result += "n"; break;
            default: break;
        }
    }
    return result;
}

uint64_t perft_original(BitboardPosition pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    BitboardMoveGen::generate_legal_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        if (pos.make_move(move)) {
            nodes += perft_original(pos, depth - 1);
            pos.unmake_move();
        }
    }
    return nodes;
}

uint64_t perft_optimized(const BitboardPosition& pos, int depth) {
    return BitboardPerftOptimized::perft(pos, depth);
}

int main() {
    cout << "=== Kiwipete Move-by-Move Analysis ===" << endl;
    
    // Initialize engine
    Huginn::init();
    if (!Huginn::is_initialized()) {
        cout << "ERROR: Failed to initialize engine" << endl;
        return 1;
    }
    
    // Set up Kiwipete position
    const string kiwipete_fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    BitboardPosition pos;
    if (!pos.set_from_fen(kiwipete_fen)) {
        cout << "Failed to parse Kiwipete position" << endl;
        return 1;
    }
    
    cout << "Kiwipete FEN: " << kiwipete_fen << endl << endl;
    
    // Generate moves with both methods
    BitboardMoveList orig_moves, opt_moves;
    BitboardMoveGen::generate_legal_moves(pos, orig_moves);
    BitboardPerftOptimized::generate_legal_moves_fast(pos, opt_moves);
    
    cout << "Move counts: Original " << orig_moves.moves.size() << ", Optimized " << opt_moves.moves.size() << endl << endl;
    
    // Convert to strings for easier comparison
    vector<string> orig_move_strings, opt_move_strings;
    for (const auto& move : orig_moves.moves) {
        orig_move_strings.push_back(move_to_string(move));
    }
    for (const auto& move : opt_moves.moves) {
        opt_move_strings.push_back(move_to_string(move));
    }
    
    sort(orig_move_strings.begin(), orig_move_strings.end());
    sort(opt_move_strings.begin(), opt_move_strings.end());
    
    // Find missing moves in optimized
    vector<string> missing_in_opt;
    set_difference(orig_move_strings.begin(), orig_move_strings.end(),
                   opt_move_strings.begin(), opt_move_strings.end(),
                   back_inserter(missing_in_opt));
    
    // Find extra moves in optimized  
    vector<string> extra_in_opt;
    set_difference(opt_move_strings.begin(), opt_move_strings.end(),
                   orig_move_strings.begin(), orig_move_strings.end(),
                   back_inserter(extra_in_opt));
    
    if (!missing_in_opt.empty()) {
        cout << "Missing in optimized (" << missing_in_opt.size() << " moves):" << endl;
        for (const string& move : missing_in_opt) {
            cout << "  " << move << endl;
        }
        cout << endl;
    }
    
    if (!extra_in_opt.empty()) {
        cout << "Extra in optimized (" << extra_in_opt.size() << " moves):" << endl;
        for (const string& move : extra_in_opt) {
            cout << "  " << move << endl;
        }
        cout << endl;
    }
    
    // Depth 2 analysis
    cout << "Move-by-move depth 2 analysis:" << endl;
    cout << "Move                Original  Optimized  Diff" << endl;
    cout << "----------------------------------------------" << endl;
    
    uint64_t total_orig = 0, total_opt = 0;
    
    for (const auto& move : orig_moves.moves) {
        BitboardPosition test_pos = pos;
        if (test_pos.make_move(move)) {
            uint64_t orig_nodes = perft_original(test_pos, 1);
            uint64_t opt_nodes = perft_optimized(test_pos, 1);
            
            total_orig += orig_nodes;
            total_opt += opt_nodes;
            
            int diff = (int)opt_nodes - (int)orig_nodes;
            cout << setw(16) << move_to_string(move) << "  " 
                 << setw(8) << orig_nodes << "  "
                 << setw(8) << opt_nodes << "  "
                 << setw(4) << diff << endl;
        }
    }
    
    cout << "----------------------------------------------" << endl;
    cout << "Total depth 2:       " << setw(8) << total_orig << "  " << setw(8) << total_opt << endl;
    cout << "Difference: " << (int)total_opt - (int)total_orig << endl;
    
    return 0;
}