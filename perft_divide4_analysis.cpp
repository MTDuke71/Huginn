#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
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

// Perft function with legal checking
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

// Move to string conversion
std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    
    std::string result;
    result += from_file;
    result += from_rank;
    result += to_file;
    result += to_rank;
    
    return result;
}

int main() {
    std::cout << "=== Perft Divide 4 Analysis ===\n\n";
    
    // Initialize position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing perft divide 4 from starting position\n\n";
    
    // Expected results for perft divide 4 (from reference engines)
    // These are the correct values we should match
    std::vector<std::pair<std::string, uint64_t>> expected_divide4 = {
        {"a2a3", 8457}, {"a2a4", 9329}, {"b1a3", 8885}, {"b1c3", 9755},
        {"b2b3", 9345}, {"b2b4", 9332}, {"c2c3", 9272}, {"c2c4", 9744},
        {"d2d3", 11959}, {"d2d4", 12435}, {"e2e3", 13134}, {"e2e4", 13160},
        {"f2f3", 8457}, {"f2f4", 8929}, {"g1f3", 9748}, {"g1h3", 8881},
        {"g2g3", 9345}, {"g2g4", 9328}, {"h2h3", 8457}, {"h2h4", 9329}
    };
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t total_our = 0;
    uint64_t total_expected = 0;
    
    std::cout << std::left << std::setw(8) << "Move" 
              << std::setw(12) << "Our Count" 
              << std::setw(12) << "Expected"
              << std::setw(12) << "Difference"
              << std::setw(10) << "Status" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = move_to_string(simple_move);
            
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t our_count = perft_legal(pos, 3);  // depth 4 = 1 + 3
            pos.unmake_move(simple_move, undo);
            
            // Find expected count
            uint64_t expected_count = 0;
            bool found = false;
            for (const auto& expected : expected_divide4) {
                if (expected.first == move_str) {
                    expected_count = expected.second;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                std::cout << "⚠️  Move " << move_str << " not found in expected results!\n";
                continue;
            }
            
            int64_t difference = (int64_t)our_count - (int64_t)expected_count;
            bool correct = (our_count == expected_count);
            std::string status = correct ? "✓" : "✗";
            
            std::cout << std::left << std::setw(8) << move_str
                      << std::setw(12) << our_count
                      << std::setw(12) << expected_count  
                      << std::setw(12) << difference
                      << std::setw(10) << status << std::endl;
            
            total_our += our_count;
            total_expected += expected_count;
        }
    }
    
    std::cout << std::string(60, '-') << std::endl;
    std::cout << std::left << std::setw(8) << "TOTAL"
              << std::setw(12) << total_our
              << std::setw(12) << total_expected
              << std::setw(12) << ((int64_t)total_our - (int64_t)total_expected)
              << std::setw(10) << (total_our == total_expected ? "✓" : "✗") << std::endl;
    
    std::cout << "\n=== Summary ===\n";
    std::cout << "Total our result:     " << total_our << "\n";
    std::cout << "Total expected:       " << total_expected << "\n";
    std::cout << "Difference:           " << ((int64_t)total_our - (int64_t)total_expected) << "\n";
    
    return 0;
}