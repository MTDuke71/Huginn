#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include <iostream>

std::string square_to_notation(int square) {
    if (square < 0 || square >= 64) return "invalid";
    int file = square % 8;
    int rank = square / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

std::string move_to_notation(const S_MOVE& move) {
    return square_to_notation(move.get_from()) + square_to_notation(move.get_to());
}

void analyze_old_engine_position(const std::string& description, const std::string& fen) {
    std::cout << "=== " << description << " (OLD ENGINE) ===\n";
    
    Position pos;
    pos.set_from_fen(fen);
    
    std::cout << "Position: " << pos.to_fen() << "\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n\n";
    
    S_MOVELIST moves;
    generate_legal_moves_enhanced(pos, moves);
    
    std::cout << "Legal moves (" << moves.count << "):\n";
    for (int i = 0; i < moves.count; i++) {
        std::cout << move_to_notation(moves.moves[i]) << "\n";
    }
    
    std::cout << "\nTotal legal moves: " << moves.count << "\n\n";
}

uint64_t old_engine_perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    S_MOVELIST moves;
    generate_legal_moves_enhanced(pos, moves);
    
    uint64_t count = 0;
    for (int i = 0; i < moves.count; i++) {
        pos.make_move_with_undo(moves.moves[i]);
        count += old_engine_perft(pos, depth - 1);
        pos.TakeMove();
        }
    
    return count;
}

void compare_perft_counts(const std::string& description, const std::string& fen) {
    std::cout << "=== " << description << " (OLD ENGINE PERFT) ===\n";
    
    Position pos;
    pos.set_from_fen(fen);
    
    std::cout << "Position: " << fen << "\n";
    
    for (int depth = 1; depth <= 2; depth++) {
        uint64_t count = old_engine_perft(pos, depth);
        std::cout << "Perft " << depth << ": " << count << "\n";
    }
    std::cout << "\n";
}

int main() {
    std::cout << "=== OLD ENGINE REFERENCE ANALYSIS ===\n\n";
    
    // Compare the positions we found problematic
    analyze_old_engine_position("After 1.h2h4 h7h5", 
                               "rnbqkbnr/ppppppp1/8/7p/7P/8/PPPPPPP1/RNBQKBNR w KQkq h6 0 2");
    
    analyze_old_engine_position("After 1.h2h4 g7g5", 
                               "rnbqkbnr/pppppp1p/8/6p1/7P/8/PPPPPPP1/RNBQKBNR w KQkq g6 0 2");
    
    analyze_old_engine_position("After 1.h2h4 e7e5", 
                               "rnbqkbnr/pppp1ppp/8/4p3/7P/8/PPPPPPP1/RNBQKBNR w KQkq e6 0 2");
    
    std::cout << "=== PERFT COMPARISON ===\n\n";
    
    compare_perft_counts("After h2h4 (Black to move)", 
                        "rnbqkbnr/pppppppp/8/8/7P/8/PPPPPPP1/RNBQKBNR b KQkq h3 0 1");
    
    compare_perft_counts("After 1.h2h4 h7h5 (White to move)", 
                        "rnbqkbnr/ppppppp1/8/7p/7P/8/PPPPPPP1/RNBQKBNR w KQkq h6 0 2");
    
    compare_perft_counts("After 1.h2h4 g7g5 (White to move)", 
                        "rnbqkbnr/pppppp1p/8/6p1/7P/8/PPPPPPP1/RNBQKBNR w KQkq g6 0 2");
    
    return 0;
}