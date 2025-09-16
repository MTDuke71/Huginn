#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <unordered_map>

using namespace BitboardMoveGen;

// Convert function
SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.is_capture = move.is_capture;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    simple_move.is_promotion = move.is_promotion;
    simple_move.promotion_type = move.promotion_type;
    return simple_move;
}

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

// Perft divide function
uint64_t perft_divide(BitboardPosition& pos, int depth, const std::string& move_prefix = "", bool print_moves = false) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;
    std::unordered_map<std::string, uint64_t> move_counts;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);

        // Make move and recurse
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        uint64_t nodes = perft_divide(pos, depth - 1);
        pos.unmake_move(simple_move, undo);

        move_counts[move_str] = nodes;
        total_nodes += nodes;
    }

    if (print_moves) {
        // Print in sorted order
        std::vector<std::pair<std::string, uint64_t>> sorted_moves(move_counts.begin(), move_counts.end());
        std::sort(sorted_moves.begin(), sorted_moves.end());

        for (const auto& pair : sorted_moves) {
            std::cout << pair.first << " - " << pair.second << "\n";
        }
    }

    return total_nodes;
}

int main() {
    std::cout << "=== Analyzing d2c1 Move - Depth 3 Results ===\n\n";

    // Expected breakdown after d2c1
    std::unordered_map<std::string, uint64_t> expected = {
        {"a6b5", 2002}, {"a6b7", 1970}, {"a6c4", 1960}, {"a6c8", 1696}, {"a6d3", 1949}, {"a6e2", 1820},
        {"a8b8", 2002}, {"a8c8", 1865}, {"a8d8", 1867}, {"b4b3", 2085}, {"b4c3", 1911}, {"b6a4", 1904},
        {"b6c4", 1871}, {"b6c8", 1680}, {"b6d5", 1856}, {"c7c5", 1903}, {"c7c6", 1995}, {"d7d6", 1918},
        {"e6d5", 1999}, {"e7c5", 2311}, {"e7d6", 2021}, {"e7d8", 1815}, {"e7f8", 1810}, {"e8c8", 1879},
        {"e8d8", 1830}, {"e8f8", 1791}, {"e8g8", 1820}, {"f6d5", 2094}, {"f6e4", 2411}, {"f6g4", 2173},
        {"f6g8", 1964}, {"f6h5", 2053}, {"f6h7", 1963}, {"g6g5", 1910}, {"g7f8", 1772}, {"g7h6", 1977},
        {"h3g2", 2245}, {"h8f8", 1637}, {"h8g8", 1727}, {"h8h4", 1991}, {"h8h5", 1955}, {"h8h6", 1817}, {"h8h7", 1818}
    };

    // Kiwipete position FEN
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Starting position: " << fen << "\n";

    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }

    // Make the d2c1 move (d2 = square 11, c1 = square 2)
    SimpleBitboardMove d2c1_move;
    d2c1_move.from_64 = 11;  // d2
    d2c1_move.to_64 = 2;     // c1
    d2c1_move.is_capture = false;
    d2c1_move.is_ep_capture = false;
    d2c1_move.is_castling = false;
    d2c1_move.is_promotion = false;
    d2c1_move.promotion_type = PieceType::Empty;

    std::cout << "Making move: d2c1\n\n";
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(d2c1_move);

    // Run depth 3 perft and collect results
    std::cout << "=== OUR ACTUAL RESULTS ===\n";
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_actual = 0;
    std::unordered_map<std::string, uint64_t> actual_results;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);

        BitboardPosition::UndoInfo move_undo = pos.make_move_with_undo(simple_move);
        uint64_t nodes = perft_divide(pos, 2);  // depth 3 total, so depth 2 recursion
        pos.unmake_move(simple_move, move_undo);

        actual_results[move_str] = nodes;
        total_actual += nodes;
        std::cout << move_str << " - " << nodes << "\n";
    }

    std::cout << "\nTotal actual: " << total_actual << "\n";
    std::cout << "Total expected: 83037\n";
    std::cout << "Difference: " << (int64_t)total_actual - 83037 << "\n\n";

    // Compare with expected results
    std::cout << "=== DISCREPANCIES ===\n";
    bool found_discrepancies = false;

    // Check for missing moves
    for (const auto& expected_move : expected) {
        if (actual_results.find(expected_move.first) == actual_results.end()) {
            std::cout << "MISSING: " << expected_move.first << " (expected " << expected_move.second << ")\n";
            found_discrepancies = true;
        } else if (actual_results[expected_move.first] != expected_move.second) {
            std::cout << "MISMATCH: " << expected_move.first
                      << " - Expected: " << expected_move.second
                      << ", Actual: " << actual_results[expected_move.first]
                      << " (diff: " << (int64_t)actual_results[expected_move.first] - (int64_t)expected_move.second << ")\n";
            found_discrepancies = true;
        }
    }

    // Check for extra moves
    for (const auto& actual_move : actual_results) {
        if (expected.find(actual_move.first) == expected.end()) {
            std::cout << "EXTRA: " << actual_move.first << " (" << actual_move.second << ")\n";
            found_discrepancies = true;
        }
    }

    if (!found_discrepancies) {
        std::cout << "✓ All moves match expected results!\n";
    }

    return 0;
}