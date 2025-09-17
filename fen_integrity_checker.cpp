#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include <iostream>
#include <string>

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

// Function to verify FEN integrity after move/unmake cycles
bool verify_fen_integrity(const std::string& original_fen, const std::string& move_name = "") {
    BitboardPosition pos;
    pos.set_from_fen(original_fen);

    std::string initial_fen = pos.to_fen();

    // Generate all moves (both legal and illegal)
    BitboardMoveList moves;
    generate_all_moves(pos, moves);

    bool all_passed = true;
    int move_count = 0;

    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string current_move_str = move_to_string(simple_move);

        // Skip if we're testing a specific move and this isn't it
        if (!move_name.empty() && current_move_str != move_name) {
            continue;
        }

        move_count++;

        // Store the original position
        std::string before_move_fen = pos.to_fen();

        // Make the move
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
        std::string after_move_fen = pos.to_fen();

        // Do some perft computation (this might trigger the bug)
        if (!move_name.empty()) {
            // Only do perft for specific moves to avoid too much output
            BitboardMoveList test_moves;
            generate_legal_moves(pos, test_moves);
            // Just count moves to trigger move generation
        }

        // Unmake the move
        pos.unmake_move(simple_move, undo_info);
        std::string after_unmake_fen = pos.to_fen();

        // Check if position is correctly restored
        if (before_move_fen != after_unmake_fen) {
            std::cout << "❌ FEN INTEGRITY FAILURE!" << std::endl;
            std::cout << "Move: " << current_move_str << std::endl;
            std::cout << "Move details: from=" << simple_move.from_64
                      << " to=" << simple_move.to_64
                      << " capture=" << simple_move.is_capture
                      << " ep=" << simple_move.is_ep_capture
                      << " castle=" << simple_move.is_castling
                      << " promo=" << simple_move.is_promotion << std::endl;
            std::cout << "Before move: " << before_move_fen << std::endl;
            std::cout << "After move:  " << after_move_fen << std::endl;
            std::cout << "After unmake:" << after_unmake_fen << std::endl;

            // Highlight the differences
            std::cout << "\nDifferences:" << std::endl;
            size_t min_len = std::min(before_move_fen.length(), after_unmake_fen.length());
            for (size_t i = 0; i < min_len; ++i) {
                if (before_move_fen[i] != after_unmake_fen[i]) {
                    std::cout << "Position " << i << ": '" << before_move_fen[i]
                              << "' -> '" << after_unmake_fen[i] << "'" << std::endl;
                }
            }
            std::cout << std::endl;

            all_passed = false;

            // If testing specific move, stop here
            if (!move_name.empty()) {
                return false;
            }
        } else {
            if (!move_name.empty()) {
                std::cout << "✅ Move " << current_move_str << " passed FEN integrity check" << std::endl;
            }
        }
    }

    if (move_name.empty()) {
        std::cout << "Tested " << move_count << " moves from position: " << original_fen << std::endl;
        if (all_passed) {
            std::cout << "✅ All moves passed FEN integrity check" << std::endl;
        } else {
            std::cout << "❌ Some moves failed FEN integrity check" << std::endl;
        }
    }

    return all_passed;
}

int main() {
    std::cout << "=== FEN Integrity Checker ===" << std::endl;

    // Test 1: Starting position (should pass)
    std::cout << "\n--- Test 1: Starting Position ---" << std::endl;
    verify_fen_integrity("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Test 2: Kiwipete position (where we found the bug)
    std::cout << "\n--- Test 2: Kiwipete Position ---" << std::endl;
    verify_fen_integrity("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    // Test 3: Specific e5f7 move that we know is problematic
    std::cout << "\n--- Test 3: Specific e5f7 Move ---" << std::endl;
    verify_fen_integrity("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", "e5f7");

    // Test 4: Specific e5g6 move
    std::cout << "\n--- Test 4: Specific e5g6 Move ---" << std::endl;
    verify_fen_integrity("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", "e5g6");

    return 0;
}