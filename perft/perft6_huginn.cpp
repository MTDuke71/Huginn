// Huginn Perft6 Performance Benchmark
// This tests the perft6 result for the starting position using VICE-style move validation.
// Expected result: 119,060,324 nodes
//
// Run with: ./build/bin/perft6_huginn
// For performance comparison, measure the time and nodes per second (nps).

#include <iostream>
#include <chrono>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "init.hpp"

// VICE-style perft function using MakeMove/TakeMove for legality validation
// This approach generates pseudo-legal moves and validates during make_move.
static uint64_t perft_vice(Position& pos, int depth) {
    if (depth == 0) return 1;
    S_MOVELIST list;
    generate_all_moves(pos, list);  // Generate pseudo-legal moves
    uint64_t nodes = 0;
    for (int i = 0; i < list.count; i++) {
        const auto& m = list.moves[i];
        // MakeMove validates legality - returns 1 if legal, 0 if illegal
        if (pos.MakeMove(m) == 1) {
            nodes += perft_vice(pos, depth - 1);
            pos.TakeMove();
        }
    }
    return nodes;
}

int main() {
    Huginn::init();
    Position pos;
    
    // Standard chess starting position FEN
    const std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    if (!pos.set_from_fen(start_fen)) {
        std::cerr << "Failed to parse starting FEN!" << std::endl;
        return 1;
    }
    
    const int depth = 6;
    const uint64_t expected_nodes = 119060324;
    
    std::cout << "Huginn Perft6 Performance Benchmark" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << "Position: Starting position" << std::endl;
    std::cout << "Depth: " << depth << std::endl;
    std::cout << "Expected nodes: " << expected_nodes << std::endl;
    std::cout << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    uint64_t nodes = perft_vice(pos, depth);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    uint64_t nps = (duration_ms > 0) ? (nodes * 1000 / duration_ms) : 0;
    
    std::cout << "Results:" << std::endl;
    std::cout << "  Nodes: " << nodes << std::endl;
    std::cout << "  Time: " << duration_ms << " ms" << std::endl;
    std::cout << "  NPS: " << nps << " nodes/second" << std::endl;
    std::cout << std::endl;
    
    if (nodes == expected_nodes) {
        std::cout << "✓ PASS: Perft6 result is correct!" << std::endl;
        return 0;
    } else {
        std::cout << "✗ FAIL: Expected " << expected_nodes << " but got " << nodes << std::endl;
        return 1;
    }
}
