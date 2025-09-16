/**
 * @file bitboard_perft.cpp
 * @brief Pure bitboard perft implementation
 * 
 * Perft testing using native 64-square BitboardPosition and BitboardMoveList.
 * Validates move generation accuracy and demonstrates make/unmake performance.
 * 
 * @author MTDuke71
 * @version 1.0
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace BitboardMoveGen;

// Utility function to convert BitboardMoveList::BitboardMove to SimpleBitboardMove
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

// ============================================================================
// PURE BITBOARD PERFT IMPLEMENTATION
// ============================================================================

/**
 * @brief Pure bitboard perft function
 * 
 * Counts all legal move paths from a given position to a specified depth.
 * Uses native BitboardPosition and BitboardMoveList for maximum performance.
 */
uint64_t perft_bitboard(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        // Convert move and check if it's legal
        SimpleBitboardMove simple_move = convert_move(move);
        
        // Only count legal moves
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            nodes += perft_bitboard(pos, depth - 1);
            pos.unmake_move(simple_move, undo_info);
        }
    }
    
    return nodes;
}

/**
 * @brief Perft with move breakdown for debugging
 */
void perft_detailed(BitboardPosition& pos, int depth, const std::string& move_history = "") {
    if (depth == 0) {
        std::cout << move_history << ": 1" << std::endl;
        return;
    }
    
    if (depth == 1) {
        // At depth 1, just count legal moves
        BitboardMoveList moves;
        generate_all_moves(pos, moves);
        
        int legal_count = 0;
        for (const auto& move : moves.moves) {
            SimpleBitboardMove simple_move = convert_move(move);
            if (pos.is_legal_move(simple_move)) {
                legal_count++;
            }
        }
        std::cout << move_history << ": " << legal_count << std::endl;
        return;
    }
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    for (const auto& move : moves.moves) {
        // Create move notation (simple)
        std::string move_notation = std::string(1, 'a' + file_of_64(move.from_64)) + 
                                   std::string(1, '1' + rank_of_64(move.from_64)) +
                                   std::string(1, 'a' + file_of_64(move.to_64)) + 
                                   std::string(1, '1' + rank_of_64(move.to_64));
        
        SimpleBitboardMove simple_move = convert_move(move);
        
        // Only process legal moves
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            
            uint64_t subnodes = perft_bitboard(pos, depth - 1);
            std::cout << move_notation << ": " << subnodes << std::endl;
            pos.unmake_move(simple_move, undo_info);
        }
    }
}

// ============================================================================
// PERFT TEST SUITE
// ============================================================================

struct PerftTest {
    std::string name;
    std::string fen;
    std::vector<std::pair<int, uint64_t>> expected_results;
};

void run_perft_test(const PerftTest& test) {
    std::cout << "Testing: " << test.name << std::endl;
    std::cout << "FEN: " << test.fen << std::endl;
    
    BitboardPosition pos;
    if (!pos.set_from_fen(test.fen)) {
        std::cout << "Error: Failed to parse FEN" << std::endl;
        return;
    }
    
    for (const auto& [depth, expected] : test.expected_results) {
        auto start = std::chrono::high_resolution_clock::now();
        
        uint64_t result = perft_bitboard(pos, depth);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        bool correct = (result == expected);
        std::cout << "Depth " << depth << ": " << result;
        if (expected > 0) {
            std::cout << " (expected: " << expected << ")";
            std::cout << (correct ? " ✓" : " ✗");
        }
        std::cout << " [" << duration.count() << " ms]" << std::endl;
        
        if (!correct && expected > 0) {
            std::cout << "FAILED! Running detailed perft..." << std::endl;
            perft_detailed(pos, std::min(depth, 2));
        }
    }
    
    std::cout << std::endl;
}

// ============================================================================
// MAIN PERFT TESTER
// ============================================================================

int main() {
    std::cout << "Pure Bitboard Perft Test Suite" << std::endl;
    std::cout << "===============================" << std::endl << std::endl;
    
    // Standard perft test positions
    std::vector<PerftTest> tests = {
        {
            "Starting Position",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            {{1, 20}, {2, 400}, {3, 8902}, {4, 197281}, {5, 4865609}, {6, 119060324}}
        },
        {
            "Kiwipete Position", 
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            {{1, 48}, {2, 2039}, {3, 97862}}
        },
        {
            "Position 3",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            {{1, 14}, {2, 191}, {3, 2812}, {4, 43238}}
        },
        {
            "Position 4",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            {{1, 48}, {2, 2039}}
        },
        {
            "Simple Endgame",
            "8/8/8/3k4/8/8/3K4/1R1Q4 w - - 0 1",
            {{1, 0}, {2, 0}}  // Will determine correct values
        }
    };
    
    for (const auto& test : tests) {
        run_perft_test(test);
    }
    
    std::cout << "Perft testing complete!" << std::endl;
    
    return 0;
}