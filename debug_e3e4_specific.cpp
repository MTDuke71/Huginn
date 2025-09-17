#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== Debugging e3e4 Move Specifically ===" << std::endl;
    
    // Initialize
    init_attack_tables();
    
    // Position after d7d6 a2a3: rnbqkbnr/ppp1pppp/3p4/8/8/P3P3/1PPP1PPP/RNBQKBNR w KQkq - 0 2
    std::string fen = "rn1qkbnr/ppp1pppp/3pb3/8/8/P3P3/1PPP1PPP/RNBQKBNR w KQkq - 1 3";
    BitboardPosition pos;
    pos.set_from_fen(fen);
    
    std::cout << "FEN: " << fen << std::endl;
    std::cout << "White to move" << std::endl;
    
    // Get moves using original method
    MoveList original_moves;
    generate_moves_bitboard_pure(pos, original_moves);
    std::cout << "\nOriginal method moves: " << original_moves.moves.size() << std::endl;
    
    bool found_e3e4_original = false;
    for (const auto& move : original_moves.moves) {
        if (move.from_square == 20 && move.to_square == 28) { // e3 to e4
            found_e3e4_original = true;
            std::cout << "✓ Found e3e4 in original" << std::endl;
            break;
        }
    }
    if (!found_e3e4_original) {
        std::cout << "✗ e3e4 NOT found in original!" << std::endl;
    }
    
    // Get moves using optimized method
    MoveList optimized_moves;
    generate_moves_optimized(pos, optimized_moves);
    std::cout << "\nOptimized method moves: " << optimized_moves.moves.size() << std::endl;
    
    bool found_e3e4_optimized = false;
    for (const auto& move : optimized_moves.moves) {
        if (move.from_square == 20 && move.to_square == 28) { // e3 to e4
            found_e3e4_optimized = true;
            std::cout << "✓ Found e3e4 in optimized" << std::endl;
            break;
        }
    }
    if (!found_e3e4_optimized) {
        std::cout << "✗ e3e4 NOT found in optimized!" << std::endl;
    }
    
    // Debug pawn positions specifically
    uint64_t white_pawns = pos.get_pieces(Color::White, PieceType::Pawn);
    std::cout << "\nWhite pawns bitboard: 0x" << std::hex << white_pawns << std::dec << std::endl;
    
    // Check if e3 pawn exists
    uint64_t e3_mask = 1ULL << 20; // e3 square
    if (white_pawns & e3_mask) {
        std::cout << "✓ Pawn on e3 exists" << std::endl;
    } else {
        std::cout << "✗ No pawn on e3!" << std::endl;
    }
    
    // Check if e4 is empty
    uint64_t occupied = pos.get_occupied();
    uint64_t e4_mask = 1ULL << 28; // e4 square
    if (!(occupied & e4_mask)) {
        std::cout << "✓ e4 square is empty" << std::endl;
    } else {
        std::cout << "✗ e4 square is occupied!" << std::endl;
    }
    
    // Manual check of single pawn push generation
    std::cout << "\n=== Manual Pawn Push Check ===" << std::endl;
    uint64_t empty = ~occupied;
    uint64_t single_pushes = (white_pawns << 8) & empty;
    std::cout << "Single pushes bitboard: 0x" << std::hex << single_pushes << std::dec << std::endl;
    
    if (single_pushes & e4_mask) {
        std::cout << "✓ e3->e4 push detected in bitboard" << std::endl;
    } else {
        std::cout << "✗ e3->e4 push NOT detected in bitboard!" << std::endl;
    }
    
    return 0;
}