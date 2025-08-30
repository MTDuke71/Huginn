// Standalone Huginn perft6 demo for direct comparison with EngineX
#include <iostream>
#include <chrono>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "init.hpp"

// VICE-style perft function
static uint64_t perft_vice(Position& pos, int depth) {
    if (depth == 0) return 1;
    S_MOVELIST list;
    generate_all_moves(pos, list);
    uint64_t nodes = 0;
    for (int i = 0; i < list.count; i++) {
        const auto& m = list.moves[i];
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
    int depth = 6;
    std::cout << "Huginn VICE-style perft demo (depth " << depth << ") on starting position" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    uint64_t nodes = perft_vice(pos, depth);
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Nodes: " << nodes << std::endl;
    std::cout << "Time: " << ms << " ms" << std::endl;
    return 0;
}
