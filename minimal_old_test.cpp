#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include <iostream>

int main() {
    std::cout << "=== MINIMAL OLD ENGINE TEST ===\n\n";
    
    try {
        Position pos;
        pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        
        std::cout << "Starting position loaded successfully\n";
        std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
        
        S_MOVELIST moves;
        generate_legal_moves_enhanced(pos, moves);
        
        std::cout << "Legal moves in starting position: " << moves.count << "\n";
        std::cout << "Expected: 20\n";
        
        if (moves.count == 20) {
            std::cout << "✅ Old engine working correctly!\n";
        } else {
            std::cout << "❌ Old engine has issues\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    } catch (...) {
        std::cout << "Unknown exception occurred\n";
    }
    
    return 0;
}