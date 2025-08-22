#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "init.hpp"

// Perft function that counts nodes at each depth
static uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    MoveList list; 
    generate_legal_moves(pos, list);
    uint64_t nodes = 0;
    for (const auto& m : list.v) {
        pos.make_move_with_undo(m);
        nodes += perft(pos, depth-1);
        pos.undo_move();
    }
    return nodes;
}

// Convert square number to algebraic notation
std::string square_to_algebraic(int square) {
    if (square < 21 || square > 98) return "??";
    
    int file = (square % 10) - 1;  // Convert from 1-8 to 0-7
    int rank = (square / 10) - 2;  // Convert from 2-9 to 0-7
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return "??";
    
    char file_char = 'a' + file;
    char rank_char = '1' + rank;
    
    return std::string(1, file_char) + std::string(1, rank_char);
}

// Perft with move breakdown - shows count for each first move
static void perft_divide(Position& pos, int depth) {
    if (depth <= 0) return;
    
    MoveList list;
    generate_legal_moves(pos, list);
    
    std::cout << "Generating moves for position: " << pos.to_fen() << std::endl;
    std::cout << "Found " << list.v.size() << " legal moves:" << std::endl;
    
    std::map<std::string, uint64_t> move_counts;
    uint64_t total = 0;
    
    for (const auto& move : list.v) {
        std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
        
        pos.make_move_with_undo(move);
        uint64_t count = (depth == 1) ? 1 : perft(pos, depth - 1);
        pos.undo_move();
        
        move_counts[move_str] = count;
        total += count;
        
        std::cout << move_str << " - " << count << std::endl;
    }
    
    std::cout << "\nTotal: " << total << std::endl;
}

int main(int argc, char* argv[]) {
    // Initialize engine subsystems
    Huginn::init();
    
    std::cout << "=== Kiwipete Position Debug Tool ===" << std::endl;
    
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cerr << "Failed to parse Kiwipete FEN!" << std::endl;
        return 1;
    }
    
    std::cout << "FEN: " << kiwipete_fen << std::endl;
    std::cout << "Parsed back: " << pos.to_fen() << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    std::cout << std::endl;
    
    // Test perft at different depths
    std::cout << "=== Perft Results ===" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    uint64_t perft1 = perft(pos, 1);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Perft(1) = " << perft1 << " (" << duration.count() << "ms)" << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    uint64_t perft2 = perft(pos, 2);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Perft(2) = " << perft2 << " (" << duration.count() << "ms)" << std::endl;
    std::cout << std::endl;
    
    // Expected results
    std::cout << "Expected: Perft(1) = 48, Perft(2) = 2039" << std::endl;
    std::cout << "Actual:   Perft(1) = " << perft1 << ", Perft(2) = " << perft2 << std::endl;
    std::cout << "Difference: Perft(1) = " << (int64_t)perft1 - 48 << ", Perft(2) = " << (int64_t)perft2 - 2039 << std::endl;
    std::cout << std::endl;
    
    // Show detailed move breakdown for depth 2
    std::cout << "=== Move Breakdown (Depth 2) ===" << std::endl;
    perft_divide(pos, 2);
    
    // Special analysis for a2a4 move
    std::cout << "\n=== Detailed Analysis of a2a4 Move ===" << std::endl;
    
    // Find the a2a4 move
    MoveList legal_moves;
    generate_legal_moves(pos, legal_moves);
    
    S_MOVE a2a4_move;
    bool found_a2a4 = false;
    
    for (const auto& move : legal_moves.v) {
        std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
        if (move_str == "a2a4") {
            a2a4_move = move;
            found_a2a4 = true;
            break;
        }
    }
    
    if (found_a2a4) {
        pos.make_move_with_undo(a2a4_move);
        
        std::cout << "Position after a2a4: " << pos.to_fen() << std::endl;
        
        MoveList moves_after_a2a4;
        generate_legal_moves(pos, moves_after_a2a4);
        
        std::cout << "Moves after a2a4 (" << moves_after_a2a4.v.size() << " total):" << std::endl;
        
        for (size_t i = 0; i < moves_after_a2a4.v.size(); ++i) {
            const auto& move = moves_after_a2a4.v[i];
            std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
            std::cout << (i+1) << ". " << move_str;
            
            if (move.is_castle()) std::cout << " (castle)";
            if (move.is_promotion()) std::cout << " (promotion)";  
            if (move.is_en_passant()) std::cout << " (en passant)";
            
            std::cout << std::endl;
        }
        
        pos.undo_move();
    } else {
        std::cout << "ERROR: Could not find a2a4 move!" << std::endl;
    }
    
    return 0;
}
