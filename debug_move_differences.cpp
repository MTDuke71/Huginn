/**
 * @file debug_move_differences.cpp
 * @brief Debug tool to compare move generation between original and optimized methods
 * 
 * This program generates and compares the exact moves produced by both methods
 * to identify discrepancies in legality checking.
 * 
 * @author MTDuke71
 * @version 1.0
 */

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"
#include "src/chess_types.hpp"

using namespace std;

// Helper function to convert move to readable string
string move_to_string(const BitboardMoveList::BitboardMove& move) {
    string result;
    
    // Convert from square (0-63) to chess notation
    auto square_to_notation = [](int square) -> string {
        char file = 'a' + (square % 8);
        char rank = '1' + (square / 8);
        return string(1, file) + string(1, rank);
    };
    
    result += square_to_notation(move.from_64);
    result += square_to_notation(move.to_64);
    
    // Add promotion piece if applicable
    if (move.promotion_type != PieceType::None) {
        switch (move.promotion_type) {
            case PieceType::Queen: result += "q"; break;
            case PieceType::Rook: result += "r"; break;
            case PieceType::Bishop: result += "b"; break;
            case PieceType::Knight: result += "n"; break;
            default: result += "?"; break;
        }
    }
    
    return result;
}

// Helper function to create move signature for comparison
string move_signature(const BitboardMoveList::BitboardMove& move) {
    string result = move_to_string(move);
    if (move.is_capture) result += "_cap";
    if (move.is_ep_capture) result += "_ep";
    if (move.is_castling) result += "_castle";
    if (move.is_promotion) result += "_promo";
    return result;
}

void compare_move_lists(const string& position_name, const char* fen) {
    cout << "\n=== Comparing Move Lists: " << position_name << " ===" << endl;
    cout << "FEN: " << fen << endl;
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        cout << "ERROR: Failed to parse FEN" << endl;
        return;
    }
    
    // Generate moves with both methods
    BitboardMoveList original_moves, optimized_moves;
    
    BitboardPosition pos_copy = pos;
    BitboardMoveGen::generate_legal_moves(pos_copy, original_moves);
    BitboardPerftOptimized::generate_legal_moves_fast(pos, optimized_moves);
    
    cout << "Original moves: " << original_moves.moves.size() << endl;
    cout << "Optimized moves: " << optimized_moves.moves.size() << endl;
    
    // Convert to sets for comparison
    set<string> original_set, optimized_set;
    
    cout << "\nOriginal moves:" << endl;
    for (size_t i = 0; i < original_moves.moves.size(); i++) {
        string sig = move_signature(original_moves.moves[i]);
        original_set.insert(sig);
        cout << "  " << (i+1) << ". " << move_to_string(original_moves.moves[i]) 
             << " (cap:" << original_moves.moves[i].is_capture 
             << " ep:" << original_moves.moves[i].is_ep_capture
             << " castle:" << original_moves.moves[i].is_castling << ")" << endl;
    }
    
    cout << "\nOptimized moves:" << endl;
    for (size_t i = 0; i < optimized_moves.moves.size(); i++) {
        string sig = move_signature(optimized_moves.moves[i]);
        optimized_set.insert(sig);
        cout << "  " << (i+1) << ". " << move_to_string(optimized_moves.moves[i]) 
             << " (cap:" << optimized_moves.moves[i].is_capture 
             << " ep:" << optimized_moves.moves[i].is_ep_capture
             << " castle:" << optimized_moves.moves[i].is_castling << ")" << endl;
    }
    
    // Find differences
    vector<string> only_in_original, only_in_optimized;
    
    set_difference(original_set.begin(), original_set.end(),
                   optimized_set.begin(), optimized_set.end(),
                   back_inserter(only_in_original));
                   
    set_difference(optimized_set.begin(), optimized_set.end(),
                   original_set.begin(), original_set.end(),
                   back_inserter(only_in_optimized));
    
    if (only_in_original.empty() && only_in_optimized.empty()) {
        cout << "\n✓ Perfect match! All moves identical." << endl;
    } else {
        cout << "\n⚠ Differences found:" << endl;
        
        if (!only_in_original.empty()) {
            cout << "\nMoves only in ORIGINAL (missing from optimized):" << endl;
            for (const string& move : only_in_original) {
                cout << "  - " << move << endl;
            }
        }
        
        if (!only_in_optimized.empty()) {
            cout << "\nMoves only in OPTIMIZED (extra moves):" << endl;
            for (const string& move : only_in_optimized) {
                cout << "  + " << move << endl;
            }
        }
    }
    
    cout << "\n" << string(60, '=') << endl;
}

int main() {
    cout << "Move Generation Comparison Tool" << endl;
    cout << "==============================" << endl;
    
    try {
        // Initialize attack tables
        Huginn::init();
        if (!Huginn::is_initialized()) {
            cout << "ERROR: Failed to initialize engine" << endl;
            return 1;
        }
        
        // Test positions
        compare_move_lists("Starting Position", 
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            
        compare_move_lists("Kiwipete", 
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
            
        compare_move_lists("Complex Endgame", 
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
        
    } catch (const exception& e) {
        cout << "Exception caught: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}