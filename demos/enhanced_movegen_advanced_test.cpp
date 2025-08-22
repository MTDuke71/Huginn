#include <iostream>
#include "../src/init.hpp"
#include "../src/position.hpp"
#include "../src/movegen_enhanced.hpp"
#include "../src/board.hpp"

void test_complex_position_scoring() {
    std::cout << "=== Complex Position Scoring Test ===" << std::endl;
    
    Position pos;
    // A position with various capture opportunities
    pos.set_from_fen("r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 4 4");
    
    std::cout << "Position with multiple capture opportunities:" << std::endl;
    print_position(pos);
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    std::cout << "\nGenerated " << moves.size() << " pseudo-legal moves" << std::endl;
    
    // Sort and show top scoring moves
    moves.sort_by_score();
    
    std::cout << "\nTop 15 moves by score:" << std::endl;
    for (int i = 0; i < std::min(15, moves.size()); ++i) {
        const S_MOVE& move = moves[i];
        std::cout << i+1 << ". Score: " << move.score 
                 << " From: " << move.get_from() 
                 << " To: " << move.get_to();
        
        if (move.is_capture()) {
            std::cout << " (Captures piece type " << static_cast<int>(move.get_captured()) << ")";
        }
        if (move.is_castle()) {
            std::cout << " (Castle)";
        }
        if (move.is_promotion()) {
            std::cout << " (Promotion to " << static_cast<int>(move.get_promoted()) << ")";
        }
        std::cout << std::endl;
    }
    
    // Test legal move generation
    S_MOVELIST legal_moves;
    generate_legal_moves_enhanced(pos, legal_moves);
    
    std::cout << "\nPseudo-legal moves: " << moves.size() << std::endl;
    std::cout << "Legal moves: " << legal_moves.size() << std::endl;
}

void test_promotion_position() {
    std::cout << "\n=== Promotion Position Test ===" << std::endl;
    
    Position pos;
    // Position with pawn promotions available
    pos.set_from_fen("8/P7/8/8/8/8/7p/8 w - - 0 1");
    
    std::cout << "Position with promotion opportunities:" << std::endl;
    print_position(pos);
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    moves.sort_by_score();
    
    std::cout << "\nAll moves (sorted by score):" << std::endl;
    for (int i = 0; i < moves.size(); ++i) {
        const S_MOVE& move = moves[i];
        std::cout << i+1 << ". Score: " << move.score 
                 << " From: " << move.get_from() 
                 << " To: " << move.get_to();
        
        if (move.is_promotion()) {
            std::cout << " (Promotion to " << static_cast<int>(move.get_promoted()) << ")";
        }
        std::cout << std::endl;
    }
}

int main() {
    // Initialize the engine
    Huginn::init();
    
    test_complex_position_scoring();
    test_promotion_position();
    
    std::cout << "\n=== Advanced Enhanced Move Generation Test Complete ===" << std::endl;
    return 0;
}
