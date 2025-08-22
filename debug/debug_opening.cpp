#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <iomanip>
#include "../src/position.hpp"
#include "movegen_enhanced.hpp"
#include "../src/init.hpp"

// Perft function that counts nodes at each depth
static uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    MoveList list; 
    generate_legal_moves(pos, list);
    uint64_t nodes = 0;
    for (const auto& m : list.v) {
        pos.make_move_with_undo(m);
        nodes += perft(pos, depth - 1);
        pos.undo_move();
    }
    return nodes;
}

// Convert square to algebraic notation
std::string square_to_algebraic(int square) {
    if (square < 21 || square > 98) return "??";
    
    int file = (square % 10) - 1;  // Convert from 1-8 to 0-7
    int rank = (square / 10) - 2;  // Convert from 2-9 to 0-7
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return "??";
    
    char file_char = 'a' + file;
    char rank_char = '1' + rank;
    
    return std::string(1, file_char) + std::string(1, rank_char);
}

int main() {
    Huginn::init();
    
    std::cout << "=== Debugging Opening Position at Depth 6 ===" << std::endl;
    
    // Set up starting position
    Position pos;
    std::string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    if (!pos.set_from_fen(starting_fen)) {
        std::cout << "Failed to parse starting FEN" << std::endl;
        return 1;
    }
    
    std::cout << "FEN: " << starting_fen << std::endl;
    
    // Expected results from depth 6
    std::map<std::string, uint64_t> expected = {
        {"a2a3", 4463267}, {"a2a4", 5363555}, {"b1a3", 4856835}, {"b1c3", 5708064},
        {"b2b3", 5310358}, {"b2b4", 5293555}, {"c2c3", 5417640}, {"c2c4", 5866666},
        {"d2d3", 8073082}, {"d2d4", 8879566}, {"e2e3", 9726018}, {"e2e4", 9771632},
        {"f2f3", 4404141}, {"f2f4", 4890429}, {"g1f3", 5723523}, {"g1h3", 4877234},
        {"g2g3", 5346260}, {"g2g4", 5239875}, {"h2h3", 4463070}, {"h2h4", 5385554}
    };
    
    uint64_t expected_total = 119060324;
    
    // Generate legal moves from starting position
    MoveList legal_moves;
    generate_legal_moves(pos, legal_moves);
    
    std::cout << "Found " << legal_moves.v.size() << " legal moves at depth 1" << std::endl;
    
    // Calculate perft for each first move
    std::cout << "\n=== Perft Breakdown (Depth 5) ===" << std::endl;
    std::cout << std::left << std::setw(8) << "Move" << std::setw(12) << "Expected" 
              << std::setw(12) << "Actual" << std::setw(12) << "Difference" << std::endl;
    std::cout << std::string(44, '-') << std::endl;
    
    uint64_t total_actual = 0;
    uint64_t total_difference = 0;
    
    for (const auto& move : legal_moves.v) {
        std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
        
        // Make the move and calculate perft(5)
        pos.make_move_with_undo(move);
        uint64_t actual_count = perft(pos, 5);  // depth 6 = 1 + 5
        pos.undo_move();
        
        total_actual += actual_count;
        
        auto it = expected.find(move_str);
        if (it != expected.end()) {
            uint64_t expected_count = it->second;
            int64_t diff = int64_t(actual_count) - int64_t(expected_count);
            total_difference += diff;
            
            std::cout << std::left << std::setw(8) << move_str
                      << std::setw(12) << expected_count
                      << std::setw(12) << actual_count
                      << std::setw(12) << diff << std::endl;
        } else {
            std::cout << std::left << std::setw(8) << move_str
                      << std::setw(12) << "UNKNOWN"
                      << std::setw(12) << actual_count
                      << std::setw(12) << "?" << std::endl;
        }
    }
    
    std::cout << std::string(44, '-') << std::endl;
    std::cout << std::left << std::setw(8) << "TOTAL"
              << std::setw(12) << expected_total
              << std::setw(12) << total_actual
              << std::setw(12) << int64_t(total_actual) - int64_t(expected_total) << std::endl;
    
    // Find moves with biggest differences
    std::cout << "\n=== Moves with Largest Differences ===" << std::endl;
    std::vector<std::pair<std::string, int64_t>> differences;
    
    for (const auto& move : legal_moves.v) {
        std::string move_str = square_to_algebraic(move.get_from()) + square_to_algebraic(move.get_to());
        
        pos.make_move_with_undo(move);
        uint64_t actual_count = perft(pos, 4);
        pos.undo_move();
        
        auto it = expected.find(move_str);
        if (it != expected.end()) {
            int64_t diff = int64_t(actual_count) - int64_t(it->second);
            if (diff != 0) {
                differences.push_back({move_str, diff});
            }
        }
    }
    
    std::sort(differences.begin(), differences.end(), 
              [](const auto& a, const auto& b) { return abs(a.second) > abs(b.second); });
    
    for (const auto& [move, diff] : differences) {
        std::cout << move << ": " << (diff > 0 ? "+" : "") << diff << std::endl;
    }
    
    return 0;
}
