#include <iostream>
#include <chrono>
#include "../src/init.hpp"
#include "../src/position.hpp"
#include "../src/movegen_enhanced.hpp"
#include "../src/board.hpp"

void test_enhanced_movegen() {
    std::cout << "=== Enhanced Move Generation Demo ===" << std::endl;
    
    Position pos;
    pos.set_startpos();
    
    std::cout << "\nStarting position:" << std::endl;
    print_position(pos);
    
    // Generate moves with enhanced system
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    std::cout << "\nGenerated " << moves.size() << " pseudo-legal moves:" << std::endl;
    
    // Display first 10 moves with scores
    for (int i = 0; i < std::min(10, moves.size()); ++i) {
        const S_MOVE& move = moves[i];
        std::cout << i+1 << ". ";
        std::cout << "From: " << move.get_from() 
                 << " To: " << move.get_to()
                 << " Score: " << move.score;
        
        if (move.is_capture()) {
            std::cout << " (Capture)";
        }
        if (move.is_castle()) {
            std::cout << " (Castle)";
        }
        if (move.is_promotion()) {
            std::cout << " (Promotion)";
        }
        if (move.is_en_passant()) {
            std::cout << " (En Passant)";
        }
        std::cout << std::endl;
    }
    
    // Test move sorting
    std::cout << "\nSorting moves by score..." << std::endl;
    moves.sort_by_score();
    
    std::cout << "Top 5 moves after sorting:" << std::endl;
    for (int i = 0; i < std::min(5, moves.size()); ++i) {
        const S_MOVE& move = moves[i];
        std::cout << i+1 << ". Score: " << move.score 
                 << " From: " << move.get_from() 
                 << " To: " << move.get_to();
        
        if (move.is_capture()) {
            std::cout << " (Capture)";
        }
        std::cout << std::endl;
    }
}

void test_legal_move_generation() {
    std::cout << "\n=== Legal Move Generation Test ===" << std::endl;
    
    Position pos;
    pos.set_startpos();
    
    S_MOVELIST pseudo_moves;
    S_MOVELIST legal_moves;
    
    generate_all_moves(pos, pseudo_moves);
    generate_legal_moves_enhanced(pos, legal_moves);
    
    std::cout << "Pseudo-legal moves: " << pseudo_moves.size() << std::endl;
    std::cout << "Legal moves: " << legal_moves.size() << std::endl;
    
    // Test with a position that has pins
    std::cout << "\nTesting position with potential pins:" << std::endl;
    pos.set_from_fen("rnbqk1nr/pppp1ppp/4p3/2b5/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 2 3");
    print_position(pos);
    
    generate_all_moves(pos, pseudo_moves);
    generate_legal_moves_enhanced(pos, legal_moves);
    
    std::cout << "Pseudo-legal moves: " << pseudo_moves.size() << std::endl;
    std::cout << "Legal moves: " << legal_moves.size() << std::endl;
}

void test_performance() {
    std::cout << "\n=== Performance Test ===" << std::endl;
    
    Position pos;
    pos.set_startpos();
    
    const int iterations = 100000;
    
    // Test pseudo-legal move generation performance
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        S_MOVELIST moves;
        generate_all_moves(pos, moves);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Pseudo-legal move generation:" << std::endl;
    std::cout << iterations << " iterations in " << duration.count() << " microseconds" << std::endl;
    std::cout << "Average: " << (double)duration.count() / iterations << " microseconds per generation" << std::endl;
    
    // Test legal move generation performance
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations / 10; ++i) {  // Legal is slower, so fewer iterations
        S_MOVELIST moves;
        generate_legal_moves_enhanced(pos, moves);
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "\nLegal move generation:" << std::endl;
    std::cout << iterations / 10 << " iterations in " << duration.count() << " microseconds" << std::endl;
    std::cout << "Average: " << (double)duration.count() / (iterations / 10) << " microseconds per generation" << std::endl;
}

void test_move_scoring() {
    std::cout << "\n=== Move Scoring Test ===" << std::endl;
    
    Position pos;
    // Position with captures available
    pos.set_from_fen("rnbqkb1r/pppp1ppp/5n2/4p3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 2 3");
    
    std::cout << "Position with capture opportunities:" << std::endl;
    print_position(pos);
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    std::cout << "\nAll moves before sorting:" << std::endl;
    for (int i = 0; i < std::min(10, moves.size()); ++i) {
        const S_MOVE& move = moves[i];
        std::cout << i+1 << ". Score: " << move.score 
                 << " From: " << move.get_from() 
                 << " To: " << move.get_to();
        
        if (move.is_capture()) {
            std::cout << " (Captures " << static_cast<int>(move.get_captured()) << ")";
        }
        std::cout << std::endl;
    }
    
    moves.sort_by_score();
    
    std::cout << "\nTop moves after sorting by score:" << std::endl;
    for (int i = 0; i < std::min(10, moves.size()); ++i) {
        const S_MOVE& move = moves[i];
        std::cout << i+1 << ". Score: " << move.score 
                 << " From: " << move.get_from() 
                 << " To: " << move.get_to();
        
        if (move.is_capture()) {
            std::cout << " (Captures " << static_cast<int>(move.get_captured()) << ")";
        }
        if (move.is_castle()) {
            std::cout << " (Castle)";
        }
        std::cout << std::endl;
    }
}

int main() {
    // Initialize the engine
    Huginn::init();
    
    test_enhanced_movegen();
    test_legal_move_generation();
    test_performance();
    test_move_scoring();
    
    std::cout << "\n=== Enhanced Move Generation Demo Complete ===" << std::endl;
    return 0;
}
