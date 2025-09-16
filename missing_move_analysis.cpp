#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <set>
#include <string>

using namespace BitboardMoveGen;

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

std::string square_to_alg(int square) {
    int file = square % 8;
    int rank = square / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

std::string move_to_string(const SimpleBitboardMove& move) {
    std::string result = square_to_alg(move.from_64) + square_to_alg(move.to_64);
    if (move.is_promotion) {
        if (move.promotion_type == PieceType::Queen) result += "q";
        else if (move.promotion_type == PieceType::Rook) result += "r";
        else if (move.promotion_type == PieceType::Bishop) result += "b";
        else if (move.promotion_type == PieceType::Knight) result += "n";
    }
    return result;
}

uint64_t perft_with_moves(BitboardPosition& pos, int depth, std::set<std::string>& all_moves) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            if (depth == 1) {
                all_moves.insert(move_to_string(simple_move));
            }
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            nodes += perft_with_moves(pos, depth - 1, all_moves);
            pos.unmake_move(simple_move, undo);
        }
    }
    
    return nodes;
}

int main() {
    std::cout << "=== Missing Move Analysis After h2h4 ===\n\n";
    
    // Set up position after h2h4
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/7P/8/PPPPPPP1/RNBQKBNR b KQkq h3 0 1");
    
    std::cout << "Position after h2h4: " << pos.to_fen() << "\n\n";
    
    // Get all legal moves at depth 1 (Black's moves)
    std::set<std::string> actual_moves;
    uint64_t actual_nodes = perft_with_moves(pos, 2, actual_moves);
    
    std::cout << "Our perft 2 result: " << actual_nodes << " (expected: 421)\n";
    std::cout << "Missing: " << (421 - actual_nodes) << " nodes\n\n";
    
    std::cout << "All legal moves from this position (" << actual_moves.size() << " moves):\n";
    for (const auto& move : actual_moves) {
        std::cout << "  " << move << "\n";
    }
    
    std::cout << "\n=== Expected Black moves from standard position ===\n";
    std::cout << "After h2h4, Black should have the same 20 opening moves:\n";
    std::cout << "Pawns: a7a6, a7a5, b7b6, b7b5, c7c6, c7c5, d7d6, d7d5, e7e6, e7e5, f7f6, f7f5, g7g6, g7g5, h7h6, h7h5\n";
    std::cout << "Knights: b8a6, b8c6, g8f6, g8h6\n";
    std::cout << "Total: 20 moves expected\n\n";
    
    if (actual_moves.size() != 20) {
        std::cout << "ERROR: Found " << actual_moves.size() << " moves instead of 20!\n";
        
        std::set<std::string> expected = {
            "a7a6", "a7a5", "b7b6", "b7b5", "c7c6", "c7c5", 
            "d7d6", "d7d5", "e7e6", "e7e5", "f7f6", "f7f5", 
            "g7g6", "g7g5", "h7h6", "h7h5",
            "b8a6", "b8c6", "g8f6", "g8h6"
        };
        
        std::cout << "\nMissing moves:\n";
        for (const auto& move : expected) {
            if (actual_moves.find(move) == actual_moves.end()) {
                std::cout << "  MISSING: " << move << "\n";
            }
        }
        
        std::cout << "\nExtra moves:\n";
        for (const auto& move : actual_moves) {
            if (expected.find(move) == expected.end()) {
                std::cout << "  EXTRA: " << move << "\n";
            }
        }
    }
    
    return 0;
}