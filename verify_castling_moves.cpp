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

void check_castling_in_position(const std::string& fen, const std::string& description) {
    std::cout << "\n=== " << description << " ===" << std::endl;
    std::cout << "FEN: " << fen << std::endl;

    BitboardPosition pos;
    pos.set_from_fen(fen);

    // Check if king is in check
    bool king_in_check = pos.is_square_attacked(pos.king_square_64[static_cast<int>(pos.side_to_move)],
                                               pos.side_to_move == Color::White ? Color::Black : Color::White);
    std::cout << "King in check: " << (king_in_check ? "YES" : "NO") << std::endl;

    // Generate legal moves
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    // Count castling moves
    std::vector<std::string> castling_moves;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove sm = convert_move(move);
        if (sm.is_castling) {
            castling_moves.push_back(move_to_string(sm));
        }
    }

    std::cout << "Castling moves available: ";
    if (castling_moves.empty()) {
        std::cout << "None" << std::endl;
    } else {
        for (size_t i = 0; i < castling_moves.size(); ++i) {
            std::cout << castling_moves[i];
            if (i < castling_moves.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << "Total legal moves: " << moves.moves.size() << std::endl;
}

int main() {
    std::cout << "=== Verifying Castling Moves in Knight Capture Positions ===" << std::endl;

    // Original position
    check_castling_in_position(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "Original Kiwipete Position"
    );

    // After e5f7
    check_castling_in_position(
        "r3k2r/p1ppqpb1/bn2pNp1/3P4/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1",
        "After e5f7 (knight captures pawn on f7)"
    );

    // After e5g6
    check_castling_in_position(
        "r3k2r/p1ppqpb1/bn2pnN1/3P4/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1",
        "After e5g6 (knight captures pawn on g6)"
    );

    return 0;
}