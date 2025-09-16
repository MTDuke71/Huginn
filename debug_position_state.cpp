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

// Debug function to check position consistency
bool check_position_consistency(const BitboardPosition& pos) {
    // Just check basic things like side to move, king positions
    if (pos.get_side_to_move() != Color::White) {
        std::cout << "ERROR: Side to move is not White after unmake!" << std::endl;
        return false;
    }
    
    // Check if we have both kings
    uint64_t white_kings = pos.get_piece_bitboard(Color::White, PieceType::King);
    uint64_t black_kings = pos.get_piece_bitboard(Color::Black, PieceType::King);
    
    if (__builtin_popcountll(white_kings) != 1) {
        std::cout << "ERROR: White king count is " << __builtin_popcountll(white_kings) << std::endl;
        return false;
    }
    
    if (__builtin_popcountll(black_kings) != 1) {
        std::cout << "ERROR: Black king count is " << __builtin_popcountll(black_kings) << std::endl;
        return false;
    }
    
    return true;
}

// Divide function with position state debugging
void divide_debug(BitboardPosition& pos, int depth) {
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::vector<std::pair<std::string, uint64_t>> results;
    uint64_t total_nodes = 0;
    
    std::cout << "Starting divide debug with " << moves.moves.size() << " moves" << std::endl;
    
    int move_num = 0;
    for (const auto& move : moves.moves) {
        move_num++;
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        std::cout << "Move " << move_num << ": " << move_str;
        
        if (pos.is_legal_move(simple_move)) {
            std::cout << " (legal)";
            
            // Check position before make_move
            if (!check_position_consistency(pos)) {
                std::cout << " - Position corrupted BEFORE make_move!" << std::endl;
                continue;
            }
            
            BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
            uint64_t nodes = (depth <= 1) ? 1 : perft_legal(pos, depth - 1);
            pos.unmake_move(simple_move, undo_info);
            
            // Check position after unmake_move
            if (!check_position_consistency(pos)) {
                std::cout << " - Position corrupted AFTER unmake_move!" << std::endl;
                std::cout << " -> " << nodes << " nodes (but position is corrupted)" << std::endl;
            } else {
                std::cout << " -> " << nodes << " nodes" << std::endl;
            }
            
            results.push_back({move_str, nodes});
            total_nodes += nodes;
        } else {
            std::cout << " (illegal)" << std::endl;
        }
        
        // Stop after the first few problem moves to see the pattern
        if (move_num == 20) break;
    }
    
    std::cout << "\nResults summary:" << std::endl;
    for (const auto& result : results) {
        std::cout << "  " << result.first << ": " << result.second << std::endl;
    }
}

int main() {
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Debug Divide with Position State Checking" << std::endl;
    std::cout << "==========================================" << std::endl;
    divide_debug(pos, 3);
    
    return 0;
}