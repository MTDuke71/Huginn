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

std::string castling_rights_to_string(uint8_t rights) {
    std::string result = "";
    if (rights & 1) result += "K";  // White kingside
    if (rights & 2) result += "Q";  // White queenside
    if (rights & 4) result += "k";  // Black kingside
    if (rights & 8) result += "q";  // Black queenside
    if (result.empty()) result = "-";
    return result;
}

void analyze_castling_after_move(const std::string& fen, const std::string& move_name) {
    std::cout << "\n=== Castling Rights Analysis: " << move_name << " ===" << std::endl;

    BitboardPosition pos;
    pos.set_from_fen(fen);

    std::cout << "Original position: " << fen << std::endl;
    std::cout << "Original castling rights: " << castling_rights_to_string(pos.castling_rights) << std::endl;

    // Find the move
    BitboardMoveList moves;
    generate_all_moves(pos, moves);

    SimpleBitboardMove target_move;
    bool found = false;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (move_to_string(simple_move) == move_name) {
            target_move = simple_move;
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "ERROR: Move " << move_name << " not found!" << std::endl;
        return;
    }

    // Make the move
    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(target_move);

    std::cout << "After " << move_name << ":" << std::endl;
    std::cout << "New position: " << pos.to_fen() << std::endl;
    std::cout << "New castling rights: " << castling_rights_to_string(pos.castling_rights) << std::endl;
    std::cout << "Stored castling rights in undo: " << castling_rights_to_string(undo_info.castling_rights) << std::endl;

    // Check for castling moves available
    BitboardMoveList next_moves;
    generate_legal_moves(pos, next_moves);

    std::vector<std::string> castling_moves;
    for (const auto& move : next_moves.moves) {
        SimpleBitboardMove sm = convert_move(move);
        if (sm.is_castling) {
            castling_moves.push_back(move_to_string(sm));
        }
    }

    std::cout << "Available castling moves: ";
    if (castling_moves.empty()) {
        std::cout << "None" << std::endl;
    } else {
        for (size_t i = 0; i < castling_moves.size(); ++i) {
            std::cout << castling_moves[i];
            if (i < castling_moves.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    // Unmake the move
    pos.unmake_move(target_move, undo_info);

    std::cout << "After unmake:" << std::endl;
    std::cout << "Restored position: " << pos.to_fen() << std::endl;
    std::cout << "Restored castling rights: " << castling_rights_to_string(pos.castling_rights) << std::endl;

    // Verify it matches original
    BitboardPosition original;
    original.set_from_fen(fen);

    if (pos.castling_rights != original.castling_rights) {
        std::cout << "ERROR: Castling rights not restored correctly!" << std::endl;
        std::cout << "Expected: " << castling_rights_to_string(original.castling_rights) << std::endl;
        std::cout << "Got: " << castling_rights_to_string(pos.castling_rights) << std::endl;
    } else {
        std::cout << "✓ Castling rights restored correctly" << std::endl;
    }
}

int main() {
    std::cout << "=== Castling Rights Debug Analysis ===" << std::endl;

    // Test the original Kiwipete position
    BitboardPosition original;
    original.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    std::cout << "\nOriginal Kiwipete castling rights: " << castling_rights_to_string(original.castling_rights) << std::endl;

    // Test both knight captures
    analyze_castling_after_move(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "e5f7"
    );

    analyze_castling_after_move(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "e5g6"
    );

    // Test a non-knight move for comparison
    analyze_castling_after_move(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "f3g3"
    );

    return 0;
}