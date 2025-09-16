#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_attacks.hpp"
#include <iostream>
#include <vector>
#include <set>
#include <string>

int main() {
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN\n";
        return 1;
    }
    
    // Generate all legal moves from root
    BitboardMoveList moves;
    BitboardMoveGen::generate_legal_moves(pos, moves);
    
    // Convert to algebraic notation
    std::set<std::string> actual_moves;
    for (auto& move : moves.moves) {
        auto sq_to_alg = [](int sq) -> std::string {
            char file = 'a' + (sq % 8);
            char rank = '1' + (sq / 8);
            return std::string(1, file) + std::string(1, rank);
        };
        std::string move_str = sq_to_alg(move.from_64) + sq_to_alg(move.to_64);
        actual_moves.insert(move_str);
    }
    
    // Expected moves from the reference
    std::set<std::string> expected_moves = {
        "a1b1", "a1c1", "a1d1", "a2a3", "a2a4", "b2b3", "c3a4", "c3b1", "c3b5", "c3d1",
        "d2c1", "d2e3", "d2f4", "d2g5", "d2h6", "d5d6", "d5e6", "e1c1", "e1d1", "e1f1",
        "e1g1", "e2a6", "e2b5", "e2c4", "e2d1", "e2d3", "e2f1", "e5c4", "e5c6", "e5d3",
        "e5d7", "e5f7", "e5g4", "e5g6", "f3d3", "f3e3", "f3f4", "f3f5", "f3f6", "f3g3",
        "f3g4", "f3h3", "f3h5", "g2g3", "g2g4", "g2h3", "h1f1", "h1g1"
    };
    
    std::cout << "Actual moves generated: " << actual_moves.size() << "\n";
    std::cout << "Expected moves: " << expected_moves.size() << "\n";
    
    // Find missing moves
    std::set<std::string> missing_moves;
    for (const auto& expected : expected_moves) {
        if (actual_moves.find(expected) == actual_moves.end()) {
            missing_moves.insert(expected);
        }
    }
    
    // Find extra moves
    std::set<std::string> extra_moves;
    for (const auto& actual : actual_moves) {
        if (expected_moves.find(actual) == expected_moves.end()) {
            extra_moves.insert(actual);
        }
    }
    
    std::cout << "\nMissing moves (" << missing_moves.size() << "):\n";
    for (const auto& move : missing_moves) {
        std::cout << "  " << move << "\n";
    }
    
    std::cout << "\nExtra moves (" << extra_moves.size() << "):\n";
    for (const auto& move : extra_moves) {
        std::cout << "  " << move << "\n";
    }
    
    std::cout << "\nActual moves:\n";
    for (const auto& move : actual_moves) {
        std::cout << move << " ";
    }
    std::cout << "\n";
    
    return 0;
}