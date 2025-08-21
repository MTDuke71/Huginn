#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include "src/position.hpp"
#include "src/movegen.hpp"
#include "src/init.hpp"
#include "src/move.hpp"

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
    std::cout << "=== Kiwipete root: Perft breakdown at depth 3 (compare) ===\n";
    Position pos;
    std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    if (!pos.set_from_fen(kiwipete_fen)) {
        std::cout << "Failed to parse Kiwipete FEN" << std::endl;
        return 1;
    }
    // Expected per-move counts
    std::map<std::string, uint64_t> expected = {
        {"a1b1", 1969}, {"a1c1", 1968}, {"a1d1", 1885}, {"a2a3", 2186}, {"a2a4", 2149},
        {"b2b3", 1964}, {"c3a4", 2203}, {"c3b1", 2038}, {"c3b5", 2138}, {"c3d1", 2040},
        {"d2c1", 1963}, {"d2e3", 2136}, {"d2f4", 2000}, {"d2g5", 2134}, {"d2h6", 2019},
        {"d5d6", 1991}, {"d5e6", 2241}, {"e1c1", 1887}, {"e1d1", 1894}, {"e1f1", 1855},
        {"e1g1", 2059}, {"e2a6", 1907}, {"e2b5", 2057}, {"e2c4", 2082}, {"e2d1", 1733},
        {"e2d3", 2050}, {"e2f1", 2060}, {"e5c4", 1880}, {"e5c6", 2027}, {"e5d3", 1803},
        {"e5d7", 2124}, {"e5f7", 2080}, {"e5g4", 1878}, {"e5g6", 1997}, {"f3d3", 2005},
        {"f3e3", 2174}, {"f3f4", 2132}, {"f3f5", 2396}, {"f3f6", 2111}, {"f3g3", 2214},
        {"f3g4", 2169}, {"f3h3", 2360}, {"f3h5", 2267}, {"g2g3", 1882}, {"g2g4", 1843},
        {"g2h3", 1970}, {"h1f1", 1929}, {"h1g1", 2013}
    };
    MoveList moves;
    generate_legal_moves(pos, moves);
    uint64_t total_actual = 0, total_expected = 0;
    std::cout << std::left << std::setw(8) << "Move" << std::setw(12) << "Expected" << std::setw(12) << "Actual" << std::setw(12) << "Diff" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    for (const auto& move : moves.v) {
        std::string alg = move_to_algebraic(move);
        pos.make_move_with_undo(move);
        uint64_t actual = perft(pos, 2); // depth 3 = 1 + 2
        pos.undo_move();
        uint64_t expected_count = expected.count(alg) ? expected[alg] : 0;
        int64_t diff = int64_t(actual) - int64_t(expected_count);
        total_actual += actual;
        total_expected += expected_count;
        std::cout << std::left << std::setw(8) << alg << std::setw(12) << expected_count << std::setw(12) << actual << std::setw(12) << diff << std::endl;
    }
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "TOTAL    " << std::setw(12) << total_expected << std::setw(12) << total_actual << std::setw(12) << (int64_t(total_actual) - int64_t(total_expected)) << std::endl;
    return 0;
}
