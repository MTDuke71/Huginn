#include "src/bitboard_perft_optimized.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <vector>
#include <set>

void debug_depth4_moves() {
    std::cout << "Debugging depth 4 extra moves...\n";

    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Generate all legal moves at depth 3 to find positions that generate wrong move counts
    BitboardMoveGen movegen;
    BitboardMoveList depth3_moves;
    movegen.generate_legal_moves(pos, depth3_moves);

    std::cout << "Found " << depth3_moves.size() << " moves at depth 3\n";

    uint64_t total_depth4_moves = 0;
    int positions_with_extra_moves = 0;

    for (size_t i = 0; i < depth3_moves.size(); i++) {
        SimpleBitboardMove move = depth3_moves.moves[i];

        // Make the move
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(move);

        // Generate moves at this position (depth 1 from here = depth 4 total)
        BitboardMoveList depth4_moves;
        movegen.generate_legal_moves(pos, depth4_moves);

        total_depth4_moves += depth4_moves.size();

        // Check if this position has an unusual number of moves
        if (depth4_moves.size() > 50) { // Most positions should have < 50 legal moves
            positions_with_extra_moves++;
            std::cout << "Position after move " << i << " has " << depth4_moves.size() << " moves\n";
            std::cout << "Move: from=" << move.from_square << " to=" << move.to_square
                      << " flags=" << static_cast<int>(move.flags) << "\n";

            // Print first few moves from this position
            std::cout << "First 10 moves from this position:\n";
            for (size_t j = 0; j < std::min((size_t)10, depth4_moves.size()); j++) {
                auto& m = depth4_moves.moves[j];
                std::cout << "  " << j << ": " << m.from_square << "->" << m.to_square
                          << " flags=" << static_cast<int>(m.flags) << "\n";
            }
            std::cout << "\n";
        }

        // Unmake the move
        pos.unmake_move(move, undo_info);
    }

    std::cout << "Total depth 4 moves: " << total_depth4_moves << " (expected: 197281)\n";
    std::cout << "Extra moves: " << (total_depth4_moves - 197281) << "\n";
    std::cout << "Positions with >50 moves: " << positions_with_extra_moves << "\n";
}

int main() {
    debug_depth4_moves();
    return 0;
}