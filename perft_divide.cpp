#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

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

// Move to string function
std::string move_to_string(const SimpleBitboardMove& move) {
    std::string result;
    result += char('a' + file_of_64(move.from_64));
    result += char('1' + rank_of_64(move.from_64));
    result += char('a' + file_of_64(move.to_64));
    result += char('1' + rank_of_64(move.to_64));
    return result;
}

// Perft WITH legal checking
uint64_t perft_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            nodes += perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo_info);
        }
    }
    
    return nodes;
}

// Divide function - shows the count for each first move
void divide(BitboardPosition& pos, int depth) {
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::vector<std::pair<std::string, uint64_t>> results;
    uint64_t total_nodes = 0;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        if (pos.is_legal_move(simple_move)) {
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            uint64_t nodes = (depth <= 1) ? 1 : perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo_info);
            
            std::string move_str = move_to_string(simple_move);
            results.push_back({move_str, nodes});
            total_nodes += nodes;
        }
    }
    
    // Sort results alphabetically for easier comparison
    std::sort(results.begin(), results.end());
    
    std::cout << "Divide " << depth << " from starting position:" << std::endl;
    std::cout << "========================================" << std::endl;
    
    for (const auto& result : results) {
        std::cout << std::setw(6) << result.first << ": " << std::setw(6) << result.second << std::endl;
    }
    
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Total: " << total_nodes << std::endl;
}

int main() {
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Bitboard Engine Perft Divide 3" << std::endl;
    std::cout << "=================================" << std::endl;
    divide(pos, 3);
    
    std::cout << "\n\nReference Perft Divide 3 (Expected):" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << "  a2a3:    380" << std::endl;
    std::cout << "  a2a4:    420" << std::endl;
    std::cout << "  b1a3:    400" << std::endl;
    std::cout << "  b1c3:    440" << std::endl;
    std::cout << "  b2b3:    420" << std::endl;
    std::cout << "  b2b4:    421" << std::endl;
    std::cout << "  c2c3:    420" << std::endl;
    std::cout << "  c2c4:    441" << std::endl;
    std::cout << "  d2d3:    539" << std::endl;
    std::cout << "  d2d4:    560" << std::endl;
    std::cout << "  e2e3:    599" << std::endl;
    std::cout << "  e2e4:    600" << std::endl;
    std::cout << "  f2f3:    380" << std::endl;
    std::cout << "  f2f4:    401" << std::endl;
    std::cout << "  g1f3:    440" << std::endl;
    std::cout << "  g1h3:    400" << std::endl;
    std::cout << "  g2g3:    420" << std::endl;
    std::cout << "  g2g4:    421" << std::endl;
    std::cout << "  h2h3:    420" << std::endl;
    std::cout << "  h2h4:    421" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Total: 8902" << std::endl;
    
    return 0;
}