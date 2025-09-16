#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace BitboardMoveGen;

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from;
    simple_move.to_64 = move.to;
    simple_move.piece = move.piece;
    simple_move.captured_piece = move.captured_piece;
    simple_move.is_capture = move.captured_piece != PIECE_NONE;
    simple_move.promotion_piece = move.promotion_piece;
    simple_move.is_promotion = move.promotion_piece != PIECE_NONE;
    simple_move.is_en_passant = move.move_type == MOVE_TYPE_EN_PASSANT;
    simple_move.is_castling = (move.move_type == MOVE_TYPE_CASTLE_KING || move.move_type == MOVE_TYPE_CASTLE_QUEEN);
    return simple_move;
}

uint64_t simple_perft(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;

    BitboardMoveList moves;
    generate_legal_moves(pos, moves);

    uint64_t total_nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        pos.make_move_with_undo(simple_move);
        total_nodes += simple_perft(pos, depth - 1);
        pos.unmake_move(simple_move);
    }
    
    return total_nodes;
}

// Test multiple positions to prove BitboardPosition works correctly
int main() {
    std::cout << "=== VERIFYING BITBOARD POSITION IS CORRECT ===" << std::endl;
    
    // Test 1: Starting position
    std::cout << "\n--- Test 1: Starting Position ---" << std::endl;
    BitboardPosition pos1;
    pos1.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    uint64_t nodes1_d1 = simple_perft(pos1, 1);
    uint64_t nodes1_d2 = simple_perft(pos1, 2);
    uint64_t nodes1_d3 = simple_perft(pos1, 3);
    uint64_t nodes1_d4 = simple_perft(pos1, 4);
    
    std::cout << "Starting position perft results:" << std::endl;
    std::cout << "Depth 1: " << nodes1_d1 << " (expected: 20)" << std::endl;
    std::cout << "Depth 2: " << nodes1_d2 << " (expected: 400)" << std::endl;
    std::cout << "Depth 3: " << nodes1_d3 << " (expected: 8902)" << std::endl;
    std::cout << "Depth 4: " << nodes1_d4 << " (expected: 197281)" << std::endl;
    
    // Test 2: Original Kiwipete position  
    std::cout << "\n--- Test 2: Original Kiwipete Position ---" << std::endl;
    BitboardPosition pos2;
    pos2.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    
    uint64_t nodes2_d1 = simple_perft(pos2, 1);
    uint64_t nodes2_d2 = simple_perft(pos2, 2);
    uint64_t nodes2_d3 = simple_perft(pos2, 3);
    
    std::cout << "Original Kiwipete perft results:" << std::endl;
    std::cout << "Depth 1: " << nodes2_d1 << " (expected: 48)" << std::endl;
    std::cout << "Depth 2: " << nodes2_d2 << " (expected: 2039)" << std::endl;
    std::cout << "Depth 3: " << nodes2_d3 << " (expected: 97862)" << std::endl;
    
    // Test 3: f3f5 position (known working)
    std::cout << "\n--- Test 3: f3f5 Position (Known Working) ---" << std::endl;
    BitboardPosition pos3;
    pos3.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PNQ2/1p2P3/2N4p/PPPBBPPP/R3K2R b KQkq - 1 1");
    
    uint64_t nodes3_d1 = simple_perft(pos3, 1);
    uint64_t nodes3_d2 = simple_perft(pos3, 2);
    uint64_t nodes3_d3 = simple_perft(pos3, 3);
    
    std::cout << "f3f5 position perft results:" << std::endl;
    std::cout << "Depth 1: " << nodes3_d1 << " (expected: 45)" << std::endl;
    std::cout << "Depth 2: " << nodes3_d2 << " (expected: 2396)" << std::endl;
    std::cout << "Depth 3: " << nodes3_d3 << " (expected: 104992)" << std::endl;
    
    // Summary
    std::cout << "\n=== SUMMARY ===" << std::endl;
    bool all_correct = true;
    
    if (nodes1_d1 == 20 && nodes1_d2 == 400 && nodes1_d3 == 8902 && nodes1_d4 == 197281) {
        std::cout << "✓ Starting position: ALL CORRECT" << std::endl;
    } else {
        std::cout << "✗ Starting position: ERRORS FOUND" << std::endl;
        all_correct = false;
    }
    
    if (nodes2_d1 == 48 && nodes2_d2 == 2039 && nodes2_d3 == 97862) {
        std::cout << "✓ Original Kiwipete: ALL CORRECT" << std::endl;
    } else {
        std::cout << "✗ Original Kiwipete: ERRORS FOUND" << std::endl;
        all_correct = false;
    }
    
    if (nodes3_d1 == 45 && nodes3_d2 == 2396 && nodes3_d3 == 104992) {
        std::cout << "✓ f3f5 position: ALL CORRECT" << std::endl;
    } else {
        std::cout << "✗ f3f5 position: ERRORS FOUND" << std::endl;
        all_correct = false;
    }
    
    if (all_correct) {
        std::cout << "\n🎉 BITBOARD POSITION IS COMPLETELY CORRECT!" << std::endl;
        std::cout << "The systematic analysis issue was NOT a BitboardPosition bug." << std::endl;
    } else {
        std::cout << "\n❌ BitboardPosition has issues that need investigation." << std::endl;
    }
    
    return 0;
}