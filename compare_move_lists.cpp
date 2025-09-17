#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/init.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <set>

using namespace BitboardMoveGen;

// Convert BitboardMoveList::BitboardMove to SimpleBitboardMove
SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.promotion_type = move.promotion_type;
    simple_move.is_capture = move.is_capture;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    simple_move.is_promotion = move.promotion_type != PieceType::None;
    return simple_move;
}

// Convert 120-square to 64-square
int square120_to_64(int sq120) {
    int rank = sq120 / 10 - 2;
    int file = sq120 % 10 - 1;
    if (rank < 0 || rank > 7 || file < 0 || file > 7) return -1;
    return rank * 8 + file;
}

// Convert 64-square to algebraic notation
std::string sq64_to_algebraic(int sq64) {
    if (sq64 < 0 || sq64 > 63) return "??";
    char file = 'a' + (sq64 % 8);
    char rank = '1' + (sq64 / 8);
    return std::string(1, file) + std::string(1, rank);
}

int main() {
    Huginn::init();

    std::cout << "=== Compare Move Lists After h7->g6 ===\n\n";

    // Position after h7->g6
    std::string after_h7g6_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q2/1PPBBPpP/R3K2R w KQkq - 0 2";
    std::cout << "Position after h7->g6: " << after_h7g6_fen << "\n\n";

    // Test with VICE
    Position vice_pos;
    vice_pos.set_from_fen(after_h7g6_fen);

    // Test with BitboardPosition
    BitboardPosition bb_pos;
    bb_pos.set_from_fen(after_h7g6_fen);

    // Get VICE moves
    S_MOVELIST vice_moves;
    generate_all_moves(vice_pos, vice_moves);
    
    // Get BitboardPosition moves
    BitboardMoveList bb_moves;
    generate_legal_moves(bb_pos, bb_moves);

    std::cout << "=== VICE Moves (All Generated: " << vice_moves.count << ") ===\n";
    std::set<std::string> vice_legal_moves;
    int vice_legal_count = 0;
    
    for (int i = 0; i < vice_moves.count; i++) {
        const auto& move = vice_moves.moves[i];
        int from_64 = square120_to_64(move.get_from());
        int to_64 = square120_to_64(move.get_to());
        
        if (from_64 == -1 || to_64 == -1) continue;
        
        std::string move_str = sq64_to_algebraic(from_64) + sq64_to_algebraic(to_64);
        
        // Test if move is legal
        Position test_pos = vice_pos;
        bool is_legal = (test_pos.MakeMove(move) == 1);
        
        if (is_legal) {
            vice_legal_moves.insert(move_str);
            vice_legal_count++;
            std::cout << "✓ " << move_str << " (" << from_64 << "->" << to_64 << ")\n";
            test_pos.TakeMove();
        } else {
            std::cout << "❌ " << move_str << " (" << from_64 << "->" << to_64 << ") ILLEGAL\n";
        }
    }
    
    std::cout << "VICE Legal moves: " << vice_legal_count << "\n\n";

    std::cout << "=== BitboardPosition Moves (Legal Only: " << bb_moves.moves.size() << ") ===\n";
    std::set<std::string> bb_legal_moves;
    
    for (const auto& move : bb_moves.moves) {
        std::string move_str = sq64_to_algebraic(move.from_64) + sq64_to_algebraic(move.to_64);
        bb_legal_moves.insert(move_str);
        std::cout << "✓ " << move_str << " (" << move.from_64 << "->" << move.to_64 << ")\n";
    }
    
    std::cout << "BitboardPosition Legal moves: " << bb_legal_moves.size() << "\n\n";

    // Find differences
    std::cout << "=== Move Set Comparison ===\n";
    
    // Moves in VICE but not in BitboardPosition
    std::cout << "In VICE but not in BitboardPosition:\n";
    bool found_diff = false;
    for (const auto& move : vice_legal_moves) {
        if (bb_legal_moves.find(move) == bb_legal_moves.end()) {
            std::cout << "- " << move << "\n";
            found_diff = true;
        }
    }
    if (!found_diff) std::cout << "(none)\n";
    
    // Moves in BitboardPosition but not in VICE
    std::cout << "\nIn BitboardPosition but not in VICE:\n";
    found_diff = false;
    for (const auto& move : bb_legal_moves) {
        if (vice_legal_moves.find(move) == vice_legal_moves.end()) {
            std::cout << "- " << move << "\n";
            found_diff = true;
        }
    }
    if (!found_diff) std::cout << "(none)\n";
    
    std::cout << "\nMove count summary:\n";
    std::cout << "VICE legal: " << vice_legal_count << "\n";
    std::cout << "BB legal: " << bb_legal_moves.size() << "\n";
    std::cout << "Should match: " << (vice_legal_count == bb_legal_moves.size() ? "✓ YES" : "❌ NO") << "\n";

    return 0;
}