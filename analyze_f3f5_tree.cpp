#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <vector>

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

// Perft function for depth 3 analysis
uint64_t perft_depth3(BitboardPosition& pos, int depth, const std::string& prefix = "") {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);

        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
        uint64_t nodes = perft_depth3(pos, depth - 1);
        pos.unmake_move(simple_move, undo);

        if (depth == 3) {
            std::cout << prefix << move_str << " - " << nodes << "\n";
        }

        total += nodes;
    }
    return total;
}

int main() {
    std::cout << "=== Analysis of Position After f3f5 ===\n\n";

    // Kiwipete position
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Starting position: " << fen << "\n\n";

    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }

    // Make the f3f5 move
    SimpleBitboardMove f3f5;
    f3f5.from_64 = 21;  // f3
    f3f5.to_64 = 37;    // f5
    f3f5.is_capture = false;
    f3f5.is_ep_capture = false;
    f3f5.is_castling = false;
    f3f5.is_promotion = false;

    std::cout << "Making move f3f5...\n";
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(f3f5);

    // Get FEN of position after f3f5
    std::string after_fen = pos.get_fen();
    std::cout << "Position after f3f5: " << after_fen << "\n\n";

    // Analyze depth 3 from this position
    std::cout << "=== DEPTH 3 ANALYSIS AFTER f3f5 ===\n";
    uint64_t total_nodes = perft_depth3(pos, 3, "  ");
    std::cout << "\nTotal nodes at depth 3: " << total_nodes << "\n";
    std::cout << "Expected from reference: 104992\n";
    std::cout << "Difference: " << (int64_t)total_nodes - 104992 << "\n";

    // Unmake the move
    pos.unmake_move(f3f5, undo);

    return 0;
}