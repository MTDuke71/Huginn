#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <map>

using namespace BitboardMoveGen;

// Convert function
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

// Convert square to algebraic notation
std::string square_to_alg_notation(int square) {
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
    std::cout << "=== Perft Divide 5 Analysis ===\n\n";
    
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Reference data from user
    std::map<std::string, uint64_t> expected = {
        {"a2a3", 181046}, {"a2a4", 217832}, {"b1a3", 198572}, {"b1c3", 234656},
        {"b2b3", 215255}, {"b2b4", 216145}, {"c2c3", 222861}, {"c2c4", 240082},
        {"d2d3", 328511}, {"d2d4", 361790}, {"e2e3", 402988}, {"e2e4", 405385},
        {"f2f3", 178889}, {"f2f4", 198473}, {"g1f3", 233491}, {"g1h3", 198502},
        {"g2g3", 217210}, {"g2g4", 214048}, {"h2h3", 181044}, {"h2h4", 218829}
    };
    
    std::cout << "Testing perft divide 5 from starting position\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t total_our = 0;
    uint64_t total_expected = 4865609;
    bool found_errors = false;
    
    std::cout << "Move    Our Count   Expected    Difference  Status\n";
    std::cout << "------------------------------------------------------------\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = square_to_alg_notation(simple_move.from_64) + 
                                   square_to_alg_notation(simple_move.to_64);
            
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t our_count = perft_legal(pos, 4);
            pos.unmake_move(simple_move, undo);
            
            auto it = expected.find(move_str);
            if (it != expected.end()) {
                uint64_t expected_count = it->second;
                int64_t diff = (int64_t)our_count - (int64_t)expected_count;
                
                std::cout << move_str << "    " << our_count << "        " << expected_count;
                std::cout << "        " << diff;
                
                if (diff == 0) {
                    std::cout << "           ✅\n";
                } else {
                    std::cout << "           ❌\n";
                    found_errors = true;
                }
                
                total_our += our_count;
            }
        }
    }
    
    std::cout << "------------------------------------------------------------\n";
    std::cout << "TOTAL   " << total_our << "      " << total_expected;
    std::cout << "      " << (int64_t)total_our - (int64_t)total_expected;
    
    if (total_our == total_expected) {
        std::cout << "           ✅\n";
    } else {
        std::cout << "           ❌\n";
    }
    
    std::cout << "\n=== Summary ===\n";
    if (!found_errors) {
        std::cout << "🎉 ALL MOVES PERFECT! No errors found at depth 5!\n";
    } else {
        std::cout << "❌ Errors found in specific moves. These moves have bugs in deeper positions.\n";
    }
    
    return 0;
}