#include <iostream>
#include <iomanip>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;

string square_to_string(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return string(1, 'a' + file) + to_string(rank + 1);
}

int main() {
    cout << "=== Debugging Specific Check Position ===" << endl;
    
    // Initialize 
    init_attack_tables();
    
    // Test one of the problematic positions: "c7c6 d2d3 d8a5"
    // FEN: rnb1kbnr/pp1ppppp/2p5/q7/8/3PP3/PPP2PPP/RNBQKBNR w KQkq - 1 3
    string fen = "rnb1kbnr/pp1ppppp/2p5/q7/8/3PP3/PPP2PPP/RNBQKBNR w KQkq - 1 3";
    BitboardPosition pos;
    pos.set_from_fen(fen);
    
    cout << "FEN: " << fen << endl;
    cout << "Position: Black queen on a5 giving check to white king on e1" << endl;
    
    // Test original method
    MoveList original_moves;
    generate_moves_bitboard_pure(pos, original_moves);
    cout << "\nOriginal method: " << original_moves.moves.size() << " moves" << endl;
    for (size_t i = 0; i < original_moves.moves.size() && i < 10; i++) {
        const auto& move = original_moves.moves[i];
        cout << "  " << square_to_string(move.from_square) << square_to_string(move.to_square) << endl;
    }
    
    // Test optimized method
    BitboardMoveList optimized_moves;
    generate_moves_optimized(pos, optimized_moves);
    cout << "\nOptimized method: " << optimized_moves.moves.size() << " moves" << endl;
    for (size_t i = 0; i < optimized_moves.moves.size() && i < 10; i++) {
        const auto& move = optimized_moves.moves[i];
        cout << "  " << square_to_string(move.from_square) << square_to_string(move.to_square) << endl;
    }
    
    // Check if king is actually in check
    Color white = Color::White;
    Color black = Color::Black;
    
    uint64_t white_king = pos.get_pieces(white, PieceType::King);
    if (white_king == 0) {
        cout << "\nERROR: No white king found!" << endl;
        return 1;
    }
    
    #ifdef _MSC_VER
        unsigned long king_square;
        _BitScanForward64(&king_square, white_king);
    #else
        int king_square = __builtin_ctzll(white_king);
    #endif
    
    cout << "\nWhite king on: " << square_to_string(king_square) << endl;
    
    // Check if the optimized code detects check correctly
    cout << "\nDebugging check detection..." << endl;
    
    return 0;
}