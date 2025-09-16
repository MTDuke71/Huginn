#include "src/bitboard_position.h"
#include "src/bitboard_movegen.h"
#include <iostream>
#include <iomanip>

// Convert square to algebraic notation
std::string square_to_alg(int square) {
    if (square == SQUARE_NONE) return "-";
    int file = square % 8;
    int rank = square / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

// Perft function
uint64_t perft_debug(BitboardPosition& pos, int depth, const std::string& path = "") {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            // Create move string
            std::string move_str = square_to_alg(simple_move.from_64) + square_to_alg(simple_move.to_64);
            
            // Capture state before move
            int en_passant_before = pos.get_en_passant_square();
            
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            
            // Capture state after move
            int en_passant_after = pos.get_en_passant_square();
            
            // Debug 2-square pawn moves specifically
            if (path.empty() && move_str.length() == 4) {
                int from_rank = simple_move.from_64 / 8;
                int to_rank = simple_move.to_64 / 8;
                bool is_pawn_2square = (from_rank == 1 && to_rank == 3) || (from_rank == 6 && to_rank == 4);
                
                if (is_pawn_2square) {
                    std::cout << "=== 2-Square Pawn Move: " << move_str << " ===\n";
                    std::cout << "En passant before: " << square_to_alg(en_passant_before) << "\n";
                    std::cout << "En passant after:  " << square_to_alg(en_passant_after) << "\n";
                    
                    // Calculate expected en passant square
                    int expected_ep = (from_rank == 1) ? simple_move.from_64 + 8 : simple_move.from_64 - 8;
                    std::cout << "Expected en passant: " << square_to_alg(expected_ep) << "\n";
                    
                    if (en_passant_after != expected_ep) {
                        std::cout << "❌ EN PASSANT SQUARE WRONG!\n";
                    } else {
                        std::cout << "✅ En passant square correct\n";
                    }
                    std::cout << "\n";
                }
            }
            
            uint64_t sub_nodes = perft_debug(pos, depth - 1, path + move_str + " ");
            pos.unmake_move(simple_move, undo);
            
            // Verify en passant restoration
            int en_passant_restored = pos.get_en_passant_square();
            if (en_passant_restored != en_passant_before) {
                std::cout << "❌ EN PASSANT RESTORATION FAILED for " << move_str << "\n";
                std::cout << "   Before: " << square_to_alg(en_passant_before) << "\n";
                std::cout << "   After unmake: " << square_to_alg(en_passant_restored) << "\n";
            }
            
            nodes += sub_nodes;
        }
    }
    
    return nodes;
}

int main() {
    std::cout << "=== En Passant Deep Debug ===\n\n";
    
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Initial en passant square: " << square_to_alg(pos.get_en_passant_square()) << "\n\n";
    
    // Test each 2-square pawn move at depth 2 to see en passant handling
    std::cout << "Testing 2-square pawn moves at depth 2:\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = square_to_alg(simple_move.from_64) + square_to_alg(simple_move.to_64);
            
            // Check if it's a 2-square pawn move
            int from_rank = simple_move.from_64 / 8;
            int to_rank = simple_move.to_64 / 8;
            bool is_pawn_2square = (from_rank == 1 && to_rank == 3);
            
            if (is_pawn_2square) {
                std::cout << "Testing " << move_str << ":\n";
                uint64_t count = perft_debug(pos, 2);
                std::cout << "  Nodes: " << count << "\n\n";
            }
        }
    }
    
    return 0;
}