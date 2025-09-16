#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace BitboardMoveGen;

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

int main() {
    std::cout << "=== Testing is_legal_move corruption ===\n\n";
    
    // Initialize position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Initial position: " << pos.to_fen() << "\n\n";
    
    // Generate all moves
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::cout << "Generated " << moves.moves.size() << " moves\n";
    
    // Test is_legal_move on first few moves to see if position gets corrupted
    std::cout << "\nTesting is_legal_move on first 5 moves:\n";
    
    for (int i = 0; i < std::min(5, (int)moves.moves.size()); i++) {
        SimpleBitboardMove simple_move = convert_move(moves.moves[i]);
        
        std::cout << "\nBefore is_legal_move call " << i+1 << ": " << pos.to_fen() << "\n";
        
        bool is_legal = pos.is_legal_move(simple_move);
        
        std::cout << "After is_legal_move call " << i+1 << ":  " << pos.to_fen() << "\n";
        std::cout << "Move is legal: " << (is_legal ? "YES" : "NO") << "\n";
        
        // Check if position changed
        if (i == 0) {
            std::string expected = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
            if (pos.to_fen() != expected) {
                std::cout << "⚠️  Position CORRUPTED after first is_legal_move call!\n";
                std::cout << "Expected: " << expected << "\n";
                std::cout << "Actual:   " << pos.to_fen() << "\n";
                break;
            }
        }
    }
    
    return 0;
}