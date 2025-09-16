/**
 * @file debug_state_corruption.cpp
 * @brief Debug state corruption in systematic analysis
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include "bitboard_attacks.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

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
        pos.unmake_move();
    }
    
    return total_nodes;
}

void print_position_checksum(const BitboardPosition& pos, const std::string& label) {
    // Print a simple checksum of key position data
    std::cout << label << " checksum:" << std::endl;
    std::cout << "  white_pieces: " << std::hex << pos.get_white_pieces() << std::dec << std::endl;
    std::cout << "  black_pieces: " << std::hex << pos.get_black_pieces() << std::dec << std::endl;
    std::cout << "  side_to_move: " << (pos.get_side_to_move() == WHITE ? "WHITE" : "BLACK") << std::endl;
    
    BitboardMoveList moves;
    generate_legal_moves(const_cast<BitboardPosition&>(pos), moves);
    std::cout << "  legal_moves: " << moves.moves.size() << std::endl;
}

int main() {
    std::cout << "=== DEBUG STATE CORRUPTION ===" << std::endl;
    
    // Test 1: Original Kiwipete systematic analysis (where corruption happens)
    std::cout << "\n--- Test 1: Original Kiwipete Systematic Analysis ---" << std::endl;
    
    BitboardPosition pos_orig;
    pos_orig.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    
    print_position_checksum(pos_orig, "Original Kiwipete initial state");
    
    // Generate moves from original position
    BitboardMoveList orig_moves;
    generate_legal_moves(pos_orig, orig_moves);
    std::cout << "Original position has " << orig_moves.moves.size() << " legal moves" << std::endl;
    
    // Find f2f3 move
    SimpleBitboardMove f2f3_move;
    bool found_f2f3 = false;
    
    for (const auto& move : orig_moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        if (move_str == "f2f3") {
            f2f3_move = simple_move;
            found_f2f3 = true;
            std::cout << "Found f2f3 move: from=" << simple_move.from_64 << " to=" << simple_move.to_64 << std::endl;
            break;
        }
    }
    
    if (!found_f2f3) {
        std::cout << "ERROR: f2f3 move not found!" << std::endl;
        return 1;
    }
    
    // Make f2f3 move 
    std::cout << "\nMaking f2f3 move..." << std::endl;
    pos_orig.make_move_with_undo(f2f3_move);
    
    print_position_checksum(pos_orig, "After f2f3 move via make_move_with_undo");
    
    // Test perft from this position
    uint64_t perft_after_f2f3 = simple_perft(pos_orig, 2);
    std::cout << "Perft depth 2 after f2f3: " << perft_after_f2f3 << std::endl;
    
    // Unmake move
    std::cout << "\nUnmaking f2f3 move..." << std::endl;
    pos_orig.unmake_move();
    
    print_position_checksum(pos_orig, "After unmaking f2f3 move");
    
    // Test 2: Direct f3f5 position (where it works correctly)
    std::cout << "\n--- Test 2: Direct f3f5 Position ---" << std::endl;
    
    BitboardPosition pos_f3f5;
    pos_f3f5.set_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PNQ2/1p2P3/2N4p/PPPBBPPP/R3K2R b KQkq - 1 1");
    
    print_position_checksum(pos_f3f5, "Direct f3f5 position");
    
    uint64_t perft_direct_f3f5 = simple_perft(pos_f3f5, 2);
    std::cout << "Perft depth 2 from direct f3f5: " << perft_direct_f3f5 << std::endl;
    
    // Test 3: Compare position states
    std::cout << "\n--- Test 3: Position State Comparison ---" << std::endl;
    
    // Reset and make f2f3 again
    pos_orig.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    pos_orig.make_move_with_undo(f2f3_move);
    
    std::cout << "Comparing position states after f2f3 move:" << std::endl;
    std::cout << "make_move_with_undo result vs direct FEN:" << std::endl;
    
    if (pos_orig.get_white_pieces() == pos_f3f5.get_white_pieces()) {
        std::cout << "✓ white_pieces match" << std::endl;
    } else {
        std::cout << "✗ white_pieces differ: " 
                  << std::hex << pos_orig.get_white_pieces() << " vs " << pos_f3f5.get_white_pieces() << std::dec << std::endl;
    }
    
    if (pos_orig.get_black_pieces() == pos_f3f5.get_black_pieces()) {
        std::cout << "✓ black_pieces match" << std::endl;
    } else {
        std::cout << "✗ black_pieces differ: " 
                  << std::hex << pos_orig.get_black_pieces() << " vs " << pos_f3f5.get_black_pieces() << std::dec << std::endl;
    }
    
    if (pos_orig.get_side_to_move() == pos_f3f5.get_side_to_move()) {
        std::cout << "✓ side_to_move matches" << std::endl;
    } else {
        std::cout << "✗ side_to_move differs" << std::endl;
    }
    
    // Generate moves from both positions and compare
    BitboardMoveList moves_after_make, moves_direct;
    generate_legal_moves(pos_orig, moves_after_make);
    generate_legal_moves(pos_f3f5, moves_direct);
    
    std::cout << "Legal moves: make_move=" << moves_after_make.moves.size() 
              << " vs direct=" << moves_direct.moves.size() << std::endl;
    
    if (moves_after_make.moves.size() != moves_direct.moves.size()) {
        std::cout << "✗ MOVE COUNT MISMATCH - This is the source of the bug!" << std::endl;
    } else {
        std::cout << "✓ Move counts match" << std::endl;
    }
    
    return 0;
}