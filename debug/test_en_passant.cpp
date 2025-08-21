#include <iostream>
#include <vector>
#include <string>
#include "../src/position.hpp"
#include "../src/movegen.hpp"
#include "../src/init.hpp"

uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    
    MoveList moves;
    generate_legal_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.v) {
        pos.make_move_with_undo(move);
        nodes += perft(pos, depth - 1);
        pos.undo_move();
    }
    return nodes;
}

std::string square_to_algebraic(int square) {
    File f = file_of(square);
    Rank r = rank_of(square);
    std::string result;
    result += char('a' + int(f));
    result += char('1' + int(r));
    return result;
}

int main() {
    Huginn::init();
    
    std::cout << "=== Testing En Passant with Test Position ===" << std::endl;
    
    // Position where en passant should be possible
    // White pawn on e5, black just played d7-d5
    Position pos;
    std::string test_fen = "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2";
    
    if (!pos.set_from_fen(test_fen)) {
        std::cout << "Failed to parse test FEN" << std::endl;
        return 1;
    }
    
    std::cout << "Test position FEN: " << test_fen << std::endl;
    std::cout << "En passant square: " << pos.ep_square << std::endl;
    
    // Generate moves and look for en passant
    MoveList moves;
    generate_legal_moves(pos, moves);
    
    std::cout << "Found " << moves.v.size() << " legal moves" << std::endl;
    
    // Look for en passant moves specifically
    std::cout << "\nAll moves:" << std::endl;
    int ep_count = 0;
    for (size_t i = 0; i < moves.v.size(); ++i) {
        const auto& move = moves.v[i];
        std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
        std::cout << (i+1) << ". " << move_str;
        if (move.is_en_passant()) {
            std::cout << " (EN PASSANT)";
            ep_count++;
        }
        if (move.is_castle()) std::cout << " (castle)";
        if (move.is_promotion()) std::cout << " (promotion)";
        std::cout << std::endl;
    }
    
    std::cout << "\nTotal en passant moves: " << ep_count << std::endl;
    
    // Test perft at depth 3 from this position
    uint64_t perft_result = perft(pos, 3);
    std::cout << "Perft(3) from this position: " << perft_result << std::endl;
    
    // Now test a simple e2e4 from starting position and see if we count nodes correctly
    std::cout << "\n=== Testing from starting position again ===" << std::endl;
    
    Position start_pos;
    std::string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    start_pos.set_from_fen(starting_fen);
    
    // Find e2e4 move
    MoveList start_moves;
    generate_legal_moves(start_pos, start_moves);
    
    S_MOVE e2e4_move;
    bool found_e2e4 = false;
    
    for (const auto& move : start_moves.v) {
        std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
        if (move_str == "e2e4") {
            e2e4_move = move;
            found_e2e4 = true;
            break;
        }
    }
    
    if (found_e2e4) {
        std::cout << "Making e2e4..." << std::endl;
        start_pos.make_move_with_undo(e2e4_move);
        
        uint64_t perft_after_e2e4 = perft(start_pos, 4);  // depth 5 total
        std::cout << "Perft(4) after e2e4: " << perft_after_e2e4 << " (expected 9771632, actual shows +29)" << std::endl;
        
        start_pos.undo_move();
    }
    
    return 0;
}
