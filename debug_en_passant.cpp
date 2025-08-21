#include <iostream>
#include <vector>
#include <string>
#include "src/position.hpp"
#include "src/movegen.hpp"
#include "src/init.hpp"

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
    
    std::cout << "=== Testing En Passant Generation ===" << std::endl;
    
    // Set up starting position
    Position pos;
    std::string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    if (!pos.set_from_fen(starting_fen)) {
        std::cout << "Failed to parse starting FEN" << std::endl;
        return 1;
    }
    
    // Test the problematic e2e4 move
    std::cout << "\n=== Testing e2e4 move ===" << std::endl;
    
    MoveList initial_moves;
    generate_legal_moves(pos, initial_moves);
    
    S_MOVE e2e4_move;
    bool found_e2e4 = false;
    
    for (const auto& move : initial_moves.v) {
        std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
        if (move_str == "e2e4") {
            e2e4_move = move;
            found_e2e4 = true;
            break;
        }
    }
    
    if (!found_e2e4) {
        std::cout << "Could not find e2e4 move!" << std::endl;
        return 1;
    }
    
    // Make e2e4
    pos.make_move_with_undo(e2e4_move);
    std::cout << "After e2e4, FEN: " << pos.to_fen() << std::endl;
    std::cout << "En passant square: " << pos.ep_square << std::endl;
    
    // Generate moves after e2e4 and look for en passant moves
    MoveList moves_after_e2e4;
    generate_legal_moves(pos, moves_after_e2e4);
    
    std::cout << "Found " << moves_after_e2e4.v.size() << " moves after e2e4" << std::endl;
    
    // Look for en passant moves specifically
    std::cout << "\nEn passant moves found:" << std::endl;
    int ep_count = 0;
    for (const auto& move : moves_after_e2e4.v) {
        if (move.is_en_passant()) {
            std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
            std::cout << "EN PASSANT: " << move_str << std::endl;
            ep_count++;
        }
    }
    
    std::cout << "Total en passant moves: " << ep_count << std::endl;
    
    // Now make a random black move and check if en passant is cleared
    std::cout << "\n=== Making a black move (a7a6) ===" << std::endl;
    
    S_MOVE a7a6_move;
    bool found_a7a6 = false;
    
    for (const auto& move : moves_after_e2e4.v) {
        std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
        if (move_str == "a7a6") {
            a7a6_move = move;
            found_a7a6 = true;
            break;
        }
    }
    
    if (found_a7a6) {
        pos.make_move_with_undo(a7a6_move);
        std::cout << "After a7a6, FEN: " << pos.to_fen() << std::endl;
        std::cout << "En passant square: " << pos.ep_square << std::endl;
        
        // Check for remaining en passant moves (should be none)
        MoveList moves_after_a7a6;
        generate_legal_moves(pos, moves_after_a7a6);
        
        int ep_count_after = 0;
        for (const auto& move : moves_after_a7a6.v) {
            if (move.is_en_passant()) {
                ep_count_after++;
            }
        }
        std::cout << "En passant moves after a7a6: " << ep_count_after << std::endl;
        
        pos.undo_move();
    }
    
    pos.undo_move();
    
    return 0;
}
