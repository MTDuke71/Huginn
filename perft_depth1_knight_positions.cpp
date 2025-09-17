#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include <iostream>
#include <chrono>

using namespace BitboardMoveGen;

uint64_t perft_bitboard(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    uint64_t nodes = 0;
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move;
        simple_move.from_64 = move.from_64;
        simple_move.to_64 = move.to_64;
        simple_move.is_capture = move.is_capture;
        simple_move.promotion_type = move.promotion_type;
        simple_move.is_promotion = move.is_promotion;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;

        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
        nodes += perft_bitboard(pos, depth - 1);
        pos.unmake_move(simple_move, undo_info);
    }

    return nodes;
}

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

void test_position_perft(const std::string& fen, const std::string& description, int depth) {
    std::cout << "\n=== " << description << " ===" << std::endl;
    std::cout << "FEN: " << fen << std::endl;

    BitboardPosition pos;
    pos.set_from_fen(fen);

    // Test at depth 1 first
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    std::cout << "Legal moves at depth 1: " << moves.moves.size() << std::endl;

    // Show first few moves for verification
    std::cout << "Sample moves: ";
    for (size_t i = 0; i < std::min(size_t(5), moves.moves.size()); ++i) {
        SimpleBitboardMove sm;
        sm.from_64 = moves.moves[i].from_64;
        sm.to_64 = moves.moves[i].to_64;
        sm.is_castling = moves.moves[i].is_castling;
        std::cout << move_to_string(sm);
        if (sm.is_castling) std::cout << "(castle)";
        if (i < std::min(size_t(5), moves.moves.size()) - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    // Run perft for specified depth
    if (depth > 1) {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t nodes = perft_bitboard(pos, depth);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Perft(" << depth << "): " << nodes << " nodes in " << duration.count() << "ms" << std::endl;
    }
}

int main() {
    std::cout << "=== Perft Analysis of Knight Capture Positions ===" << std::endl;

    // Test all three positions
    test_position_perft(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "Original Kiwipete Position",
        2
    );

    test_position_perft(
        "r3k2r/p1ppqpb1/bn2pNp1/3P4/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1",
        "After e5f7 (king in check)",
        2
    );

    test_position_perft(
        "r3k2r/p1ppqpb1/bn2pnN1/3P4/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1",
        "After e5g6 (castling available)",
        2
    );

    return 0;
}