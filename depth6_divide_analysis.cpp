#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <map>
#include <vector>
#include <string>

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

// Convert 64-square to algebraic notation
std::string square64_to_algebraic(int square_64) {
    int file = square_64 % 8;
    int rank = square_64 / 8;
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

// Convert move to string
std::string move_to_string(const BitboardMoveList::BitboardMove& move) {
    std::string result = square64_to_algebraic(move.from_64) + square64_to_algebraic(move.to_64);
    if (move.is_promotion) {
        switch (move.promotion_type) {
            case PieceType::Queen: result += "q"; break;
            case PieceType::Rook: result += "r"; break;
            case PieceType::Bishop: result += "b"; break;
            case PieceType::Knight: result += "n"; break;
            default: break;
        }
    }
    return result;
}

using namespace std;

struct MoveResult {
    std::string move;
    uint64_t our_count;
    uint64_t expected_count;
    int64_t difference;
};

int main() {
    std::cout << "=== Depth 6 Perft Divide Analysis ===" << std::endl;
    std::cout << "Comparing our results with reference data" << std::endl << std::endl;
    
    // Reference data from user
    std::vector<std::pair<std::string, uint64_t>> reference_data = {
        {"a2a3", 4463267},
        {"a2a4", 5363555},
        {"b1a3", 4856835},
        {"b1c3", 5708064},
        {"b2b3", 5310358},
        {"b2b4", 5293555},
        {"c2c3", 5417640},
        {"c2c4", 5866666},
        {"d2d3", 8073082},
        {"d2d4", 8879566},
        {"e2e3", 9726018},
        {"e2e4", 9771632},
        {"f2f3", 4404141},
        {"f2f4", 4890429},
        {"g1f3", 5723523},
        {"g1h3", 4877234},
        {"g2g3", 5346260},
        {"g2g4", 5239875},
        {"h2h3", 4463070},
        {"h2h4", 5385554}
    };
    
    // Create reference lookup
    std::map<std::string, uint64_t> expected;
    for (const auto& pair : reference_data) {
        expected[pair.first] = pair.second;
    }
    
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::vector<MoveResult> results;
    uint64_t total_our = 0;
    uint64_t total_expected = 0;
    
    std::cout << "Move     Our Result   Expected     Difference   Status" << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition temp_pos = pos;
            BitboardPosition::UndoInfo undo = temp_pos.make_move_with_undo(simple_move);
            
            auto start = std::chrono::high_resolution_clock::now();
            uint64_t count = perft_legal(temp_pos, 5);  // depth 6 = 5 moves deep
            auto end = std::chrono::high_resolution_clock::now();
            
            temp_pos.unmake_move(simple_move, undo);
            
            std::string move_str = move_to_string(move);
            uint64_t expected_count = expected[move_str];
            int64_t diff = (int64_t)count - (int64_t)expected_count;
            
            total_our += count;
            total_expected += expected_count;
            
            std::string status = (diff == 0) ? "✓ CORRECT" : 
                           (diff > 0) ? "✗ OVER" : "✗ UNDER";
            
            std::cout << std::left << std::setw(8) << move_str 
                 << " " << std::setw(11) << count
                 << " " << std::setw(11) << expected_count
                 << " " << std::setw(11) << diff
                 << " " << status << std::endl;
            
            results.push_back({move_str, count, expected_count, diff});
        }
    }
    
    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "TOTAL    " << std::setw(11) << total_our 
         << " " << std::setw(11) << total_expected
         << " " << std::setw(11) << ((int64_t)total_our - (int64_t)total_expected) << std::endl;
    
    std::cout << std::endl << "=== Problem Moves ===" << std::endl;
    for (const auto& result : results) {
        if (result.difference != 0) {
            std::cout << result.move << ": " << result.difference 
                 << " (" << result.our_count << " vs " << result.expected_count << ")" << std::endl;
        }
    }
    
    return 0;
}