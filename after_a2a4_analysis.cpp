#include "src/bitboard_position.h"
#include "src/bitboard_movegen.h"
#include <iostream>
#include <iomanip>
#include <map>

// Convert square to algebraic notation
std::string square_to_alg_notation(int square) {
    if (square == SQUARE_NONE) return "-";
    int file = square % 8;
    int rank = square / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

// Perft function
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            nodes += perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
        }
    }
    
    return nodes;
}

int main() {
    std::cout << "=== Perft Analysis After a2a4 ===\n\n";
    
    // Start from initial position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Make the a2a4 move
    SimpleBitboardMove a2a4;
    a2a4.from_64 = 8;  // a2 square (rank 1, file 0)
    a2a4.to_64 = 24;   // a4 square (rank 3, file 0)
    
    std::cout << "Making move a2a4...\n";
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(a2a4);
    
    std::cout << "En passant square after a2a4: " << square_to_alg_notation(pos.get_en_passant_square()) << "\n";
    std::cout << "Expected en passant square: a3\n\n";
    
    // Reference data from user after a2a4
    std::map<std::string, uint64_t> expected = {
        {"a7a5", 9062}, {"a7a6", 9312}, {"b7b5", 11606}, {"b7b6", 10348},
        {"b8a6", 9827}, {"b8c6", 10746}, {"c7c5", 10737}, {"c7c6", 10217},
        {"d7d5", 13725}, {"d7d6", 13203}, {"e7e5", 14560}, {"e7e6", 14534},
        {"f7f5", 9847}, {"f7f6", 9328}, {"g7g5", 10293}, {"g7g6", 10310},
        {"g8f6", 10758}, {"g8h6", 9798}, {"h7h5", 10293}, {"h7h6", 9328}
    };
    
    uint64_t expected_total = 217832;
    
    std::cout << "Testing perft divide 4 after a2a4\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t total_our = 0;
    bool found_errors = false;
    
    std::cout << "Move    Our Count   Expected    Difference  Status\n";
    std::cout << "------------------------------------------------------------\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = square_to_alg_notation(simple_move.from_64) + 
                                   square_to_alg_notation(simple_move.to_64);
            
            BitboardPosition::UndoInfo move_undo = pos.make_move_with_undo(simple_move);
            uint64_t our_count = perft_legal(pos, 3);  // depth 4 total = depth 3 after first move
            pos.unmake_move(simple_move, move_undo);
            
            auto it = expected.find(move_str);
            if (it != expected.end()) {
                uint64_t expected_count = it->second;
                int64_t diff = (int64_t)our_count - (int64_t)expected_count;
                
                std::cout << move_str << "    " << std::setw(8) << our_count << "    " << std::setw(8) << expected_count;
                std::cout << "    " << std::setw(8) << diff;
                
                if (diff == 0) {
                    std::cout << "       ✅\n";
                } else {
                    std::cout << "       ❌\n";
                    found_errors = true;
                }
                
                total_our += our_count;
            } else {
                std::cout << move_str << "    " << std::setw(8) << our_count << "    UNKNOWN     UNKNOWN        ?\n";
                total_our += our_count;
            }
        }
    }
    
    std::cout << "------------------------------------------------------------\n";
    int64_t total_diff = (int64_t)total_our - (int64_t)expected_total;
    std::cout << "TOTAL   " << std::setw(8) << total_our << "    " << std::setw(8) << expected_total;
    std::cout << "    " << std::setw(8) << total_diff;
    
    if (total_diff == 0) {
        std::cout << "       ✅\n";
    } else {
        std::cout << "       ❌\n";
        found_errors = true;
    }
    
    std::cout << "\n=== Summary ===\n";
    if (found_errors) {
        std::cout << "❌ Errors found in position after a2a4. The bug manifests here.\n";
    } else {
        std::cout << "✅ All moves match perfectly after a2a4.\n";
    }
    
    // Restore original position
    pos.unmake_move(a2a4, undo);
    std::cout << "\nPosition restored. En passant: " << square_to_alg_notation(pos.get_en_passant_square()) << "\n";
    
    return 0;
}