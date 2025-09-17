#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;

SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.is_capture = move.is_capture;
    simple_move.promotion_type = move.promotion_type;
    simple_move.is_promotion = move.is_promotion;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    return simple_move;
}

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

uint64_t simple_perft(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
        total_nodes += simple_perft(pos, depth - 1);
        pos.unmake_move(simple_move, undo_info);
    }

    return total_nodes;
}

int main() {
    std::cout << "=== Debugging E5F7 Move Specifically ===" << std::endl;

    // Start with Kiwipete position
    BitboardPosition pos;
    pos.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    std::cout << "Position: " << pos.to_fen() << std::endl;

    // Find the e5f7 move
    BitboardMoveList moves;
    generate_all_moves(pos, moves);

    SimpleBitboardMove e5f7_move;
    bool found_e5f7 = false;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);

        if (move_str == "e5f7") {
            e5f7_move = simple_move;
            found_e5f7 = true;
            std::cout << "Found e5f7 move: from=" << simple_move.from_64
                      << " to=" << simple_move.to_64
                      << " capture=" << simple_move.is_capture << std::endl;
            break;
        }
    }

    if (!found_e5f7) {
        std::cout << "ERROR: e5f7 move not found!" << std::endl;
        return 1;
    }

    // Test if move is legal
    if (!pos.is_legal_move(e5f7_move)) {
        std::cout << "ERROR: e5f7 move is not legal!" << std::endl;
        return 1;
    }

    std::cout << "e5f7 move is legal" << std::endl;

    // Make the move and test perft at different depths
    std::cout << "\nMaking e5f7 move..." << std::endl;
    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(e5f7_move);

    std::cout << "Position after e5f7: " << pos.to_fen() << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;

    // Test perft from this position
    for (int depth = 1; depth <= 3; ++depth) {
        uint64_t nodes = simple_perft(pos, depth);
        std::cout << "Perft depth " << depth << ": " << nodes << " nodes" << std::endl;
    }

    // Unmake the move
    pos.unmake_move(e5f7_move, undo_info);

    std::cout << "\nAfter unmake: " << pos.to_fen() << std::endl;

    // Verify position is restored correctly
    BitboardPosition original_pos;
    original_pos.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    bool positions_match = (pos.to_fen() == original_pos.to_fen());
    std::cout << "Position restore check: " << (positions_match ? "PASS" : "FAIL") << std::endl;

    if (!positions_match) {
        std::cout << "Expected: " << original_pos.to_fen() << std::endl;
        std::cout << "Actual:   " << pos.to_fen() << std::endl;
    }

    return 0;
}