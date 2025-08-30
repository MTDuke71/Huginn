// Minimal Huginn perft6 driver using VICE-style perft
#include <iostream>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "init.hpp"

// VICE Perft function - counts all legal move paths to a given depth using VICE MakeMove/TakeMove
static uint64_t perft_vice(Position& pos, int depth) {
    if (depth == 0) return 1;
    S_MOVELIST list;
    generate_all_moves(pos, list);  // Use pseudo-legal moves + MakeMove validation (true VICE style)
    uint64_t nodes = 0;
    for (int i = 0; i < list.count; i++) {
        const auto& m = list.moves[i];
        // Use VICE MakeMove/TakeMove - MakeMove validates legality
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
    const std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if (!pos.set_from_fen(fen)) {
        std::cerr << "Failed to parse starting FEN!" << std::endl;
        return 1;
    }
    uint64_t nodes = perft_vice(pos, 6);
    std::cout << "Perft(6) nodes: " << nodes << std::endl;
    return 0;
}
