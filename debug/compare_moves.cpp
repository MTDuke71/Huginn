#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include "position.hpp"
#include "movegen.hpp"
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

int main(int argc, char* argv[]) {
    // Initialize engine subsystems
    Huginn::init();
    
    std::cout << "=== Kiwipete Move Comparison Tool ===" << std::endl;
    
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cerr << "Failed to parse Kiwipete FEN!" << std::endl;
        return 1;
    }
    
    // Expected results from user
    std::map<std::string, uint64_t> expected = {
        {"a1b1", 43}, {"a1c1", 43}, {"a1d1", 43}, {"a2a3", 44}, {"a2a4", 44},
        {"b2b3", 42}, {"c3a4", 42}, {"c3b1", 42}, {"c3b5", 39}, {"c3d1", 42},
        {"d2c1", 43}, {"d2e3", 43}, {"d2f4", 43}, {"d2g5", 42}, {"d2h6", 41},
        {"d5d6", 41}, {"d5e6", 46}, {"e1c1", 43}, {"e1d1", 43}, {"e1f1", 43},
        {"e1g1", 43}, {"e2a6", 36}, {"e2b5", 39}, {"e2c4", 41}, {"e2d1", 44},
        {"e2d3", 42}, {"e2f1", 44}, {"e5c4", 42}, {"e5c6", 41}, {"e5d3", 43},
        {"e5d7", 45}, {"e5f7", 44}, {"e5g4", 44}, {"e5g6", 42}, {"f3d3", 42},
        {"f3e3", 43}, {"f3f4", 43}, {"f3f5", 45}, {"f3f6", 39}, {"f3g3", 43},
        {"f3g4", 43}, {"f3h3", 43}, {"f3h5", 43}, {"g2g3", 42}, {"g2g4", 42},
        {"g2h3", 43}, {"h1f1", 43}, {"h1g1", 43}
    };
    
    std::cout << "Generating our moves and comparing..." << std::endl;
    
    MoveList list;
    generate_legal_moves(pos, list);
    
    std::map<std::string, uint64_t> actual;
    uint64_t total_actual = 0;
    
    for (const auto& move : list.v) {
        std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
        
        pos.make_move_with_undo(move);
        uint64_t count = perft(pos, 1);  // Depth 1 to get depth 2 total
        pos.undo_move();
        
        actual[move_str] = count;
        total_actual += count;
    }
    
    std::cout << "\n=== COMPARISON RESULTS ===" << std::endl;
    std::cout << "Expected total: 2039" << std::endl;
    std::cout << "Actual total: " << total_actual << std::endl;
    std::cout << "Difference: " << (int64_t)total_actual - 2039 << std::endl;
    
    std::cout << "\n=== MOVE-BY-MOVE COMPARISON ===" << std::endl;
    
    // Find moves that are in expected but not in actual
    for (const auto& [move, expected_count] : expected) {
        if (actual.find(move) == actual.end()) {
            std::cout << "MISSING: " << move << " (expected " << expected_count << ")" << std::endl;
        }
    }
    
    // Find moves that are in actual but not in expected
    for (const auto& [move, actual_count] : actual) {
        if (expected.find(move) == expected.end()) {
            std::cout << "EXTRA: " << move << " (actual " << actual_count << ")" << std::endl;
        }
    }
    
    // Compare moves that are in both
    int differences = 0;
    for (const auto& [move, expected_count] : expected) {
        auto it = actual.find(move);
        if (it != actual.end()) {
            uint64_t actual_count = it->second;
            if (actual_count != expected_count) {
                std::cout << "DIFFERENT: " << move << " expected=" << expected_count 
                          << " actual=" << actual_count << " diff=" << (int64_t)actual_count - (int64_t)expected_count << std::endl;
                differences++;
            }
        }
    }
    
    if (differences == 0) {
        std::cout << "All common moves have matching counts!" << std::endl;
    } else {
        std::cout << "Found " << differences << " moves with different counts." << std::endl;
    }
    
    return 0;
}
