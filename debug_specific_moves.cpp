/**
 * @file debug_specific_moves.cpp
 * @brief Debug specific problematic moves: a2a4, h2h3, h2h4
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <string>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

void debug_move_after_position(const string& move_str, int from_64, int to_64) {
    cout << "\n=== Debugging move " << move_str << " ===" << endl;
    
    // Start with initial position
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "Failed to parse starting position" << endl;
        return;
    }
    
    // Make the move
    SimpleBitboardMove move(from_64, to_64);
    BitboardPosition pos_after = pos;
    if (!pos_after.make_move(move)) {
        cout << "Failed to make move " << move_str << endl;
        return;
    }
    
    cout << "Position after " << move_str << ":" << endl;
    cout << "Side to move: " << (pos_after.side_to_move == Color::White ? "White" : "Black") << endl;
    
    // Generate moves from both methods
    BitboardMoveList original_moves, optimized_moves;
    
    BitboardPosition pos_copy = pos_after;
    BitboardMoveGen::generate_legal_moves(pos_copy, original_moves);
    BitboardPerftOptimized::generate_legal_moves_fast(pos_after, optimized_moves);
    
    cout << "Original method:  " << original_moves.moves.size() << " moves" << endl;
    cout << "Optimized method: " << optimized_moves.moves.size() << " moves" << endl;
    
    if (original_moves.moves.size() != optimized_moves.moves.size()) {
        cout << "MISMATCH! Let's compare move lists:" << endl;
        
        // Convert to sets for easy comparison
        set<string> orig_set, opt_set;
        
        for (const auto& move : original_moves.moves) {
            string move_notation = "";
            move_notation += char('a' + (move.from_64 % 8));
            move_notation += char('1' + (move.from_64 / 8));
            move_notation += char('a' + (move.to_64 % 8));
            move_notation += char('1' + (move.to_64 / 8));
            if (move.promotion_type != PieceType::None) {
                move_notation += (move.promotion_type == PieceType::Queen) ? "q" :
                                (move.promotion_type == PieceType::Rook) ? "r" :
                                (move.promotion_type == PieceType::Bishop) ? "b" : "n";
            }
            orig_set.insert(move_notation);
        }
        
        for (const auto& move : optimized_moves.moves) {
            string move_notation = "";
            move_notation += char('a' + (move.from_64 % 8));
            move_notation += char('1' + (move.from_64 / 8));
            move_notation += char('a' + (move.to_64 % 8));
            move_notation += char('1' + (move.to_64 / 8));
            if (move.promotion_type != PieceType::None) {
                move_notation += (move.promotion_type == PieceType::Queen) ? "q" :
                                (move.promotion_type == PieceType::Rook) ? "r" :
                                (move.promotion_type == PieceType::Bishop) ? "b" : "n";
            }
            opt_set.insert(move_notation);
        }
        
        // Find differences
        cout << "\nMoves in original but not optimized:" << endl;
        for (const auto& move : orig_set) {
            if (opt_set.find(move) == opt_set.end()) {
                cout << "  " << move << endl;
            }
        }
        
        cout << "\nMoves in optimized but not original:" << endl;
        for (const auto& move : opt_set) {
            if (orig_set.find(move) == orig_set.end()) {
                cout << "  " << move << endl;
            }
        }
    } else {
        cout << "Move counts match!" << endl;
    }
}

int main() {
    cout << "Debug Specific Problematic Moves" << endl;
    cout << "================================" << endl;
    
    try {
        Huginn::init();
        if (!Huginn::is_initialized()) {
            cout << "ERROR: Failed to initialize engine" << endl;
            return 1;
        }
        
        // Debug the three problematic moves
        debug_move_after_position("a2a4", 8, 24);   // a2 = 8, a4 = 24
        debug_move_after_position("h2h3", 15, 23);  // h2 = 15, h3 = 23
        debug_move_after_position("h2h4", 15, 31);  // h2 = 15, h4 = 31
        
    } catch (const exception& e) {
        cout << "Exception: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}