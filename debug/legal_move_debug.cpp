#include <iostream>
#include "../src/position.hpp"
#include "../src/movegen_enhanced.hpp"
#include "../src/bitboard_movegen.hpp"
#include "../src/init.hpp"

int main() {
    std::cout << "Legal Move Filtering Debug\n";
    std::cout << "==========================\n";
    
    // Initialize engine
    Huginn::init();
    
    // Set up starting position
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "\n1. Testing pseudo-legal move generation:\n";
    S_MOVELIST pseudo_moves;
    BitboardMoveGen::generate_all_moves_bitboard(pos, pseudo_moves);
    std::cout << "Pseudo-legal moves: " << pseudo_moves.count << std::endl;
    
    // Count knight moves in pseudo-legal
    int pseudo_knight_count = 0;
    for (int i = 0; i < pseudo_moves.count; ++i) {
        int from = pseudo_moves.moves[i].get_from();
        if (from == 22 || from == 27) { // b1 or g1
            pseudo_knight_count++;
            std::cout << "  Pseudo knight move: from=" << from << " to=" << pseudo_moves.moves[i].get_to() << std::endl;
        }
    }
    std::cout << "Pseudo knight moves: " << pseudo_knight_count << std::endl;
    
    std::cout << "\n2. Testing legal move filtering:\n";
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    std::cout << "Legal moves: " << legal_moves.count << std::endl;
    
    // Count knight moves in legal
    int legal_knight_count = 0;
    for (int i = 0; i < legal_moves.count; ++i) {
        int from = legal_moves.moves[i].get_from();
        if (from == 22 || from == 27) { // b1 or g1
            legal_knight_count++;
            std::cout << "  Legal knight move: from=" << from << " to=" << legal_moves.moves[i].get_to() << std::endl;
        }
    }
    std::cout << "Legal knight moves: " << legal_knight_count << std::endl;
    
    std::cout << "\n3. Testing individual knight move legality:\n";
    
    // Test each pseudo-legal knight move individually
    for (int i = 0; i < pseudo_moves.count; ++i) {
        int from = pseudo_moves.moves[i].get_from();
        if (from == 22 || from == 27) { // Knight move
            Position test_pos = pos;
            S_MOVE knight_move = pseudo_moves.moves[i];
            
            std::cout << "Testing knight move from " << from << " to " << knight_move.get_to() << ": ";
            
            int result = test_pos.MakeMove(knight_move);
            if (result == 1) {
                std::cout << "✅ LEGAL" << std::endl;
                test_pos.TakeMove();
            } else {
                std::cout << "❌ ILLEGAL (MakeMove returned " << result << ")" << std::endl;
            }
        }
    }
    
    return 0;
}