#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include "../src/position.hpp"
#include "movegen_enhanced.hpp"
#include "../src/init.hpp"
#include "../src/move.hpp"

// Perft function
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

std::string move_to_algebraic(const S_MOVE& move) {
    File from_file = file_of(move.get_from());
    Rank from_rank = rank_of(move.get_from());
    File to_file = file_of(move.get_to());
    Rank to_rank = rank_of(move.get_to());
    std::string result;
    result += char('a' + int(from_file));
    result += char('1' + int(from_rank));
    result += char('a' + int(to_file));
    result += char('1' + int(to_rank));
    return result;
}

int main() {
    Huginn::init();
    std::cout << "=== Kiwipete root: Perft breakdown at depth 3 ===\n";
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cout << "Failed to parse Kiwipete FEN" << std::endl;
        return 1;
    }
    MoveList moves;
    generate_legal_moves(pos, moves);
    uint64_t total_actual = 0;
    std::cout << std::left << std::setw(8) << "Move" << std::setw(12) << "Actual" << std::endl;
    std::cout << "-------------------------" << std::endl;
    for (const auto& move : moves.v) {
        std::string alg = move_to_algebraic(move);
        pos.make_move_with_undo(move);
        uint64_t actual = perft(pos, 2); // depth 3 = 1 + 2
        pos.undo_move();
        total_actual += actual;
        std::cout << std::left << std::setw(8) << alg << std::setw(12) << actual << std::endl;
    }
    std::cout << "-------------------------" << std::endl;
    std::cout << "TOTAL    " << std::setw(12) << total_actual << std::endl;
    return 0;
}
