#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

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

// Function to check if position is still valid
bool verify_starting_position(const BitboardPosition& pos) {
    // Check that we have 16 pieces for each side
    uint64_t white_pieces = 0, black_pieces = 0;
    
    for (int piece_type = 1; piece_type < 7; ++piece_type) {
        white_pieces += __builtin_popcountll(pos.get_piece_bitboard_const(Color::White, static_cast<PieceType>(piece_type)));
        black_pieces += __builtin_popcountll(pos.get_piece_bitboard_const(Color::Black, static_cast<PieceType>(piece_type)));
    }
    
    if (white_pieces != 16 || black_pieces != 16) {
        std::cout << "ERROR: Piece count wrong - White: " << white_pieces << ", Black: " << black_pieces << std::endl;
        return false;
    }
    
    // Check side to move
    if (pos.get_side_to_move_const() != Color::White) {
        std::cout << "ERROR: Side to move is not White" << std::endl;
        return false;
    }
    
    return true;
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

int main() {
    std::cout << "Testing single make/unmake cycles to isolate corruption:" << std::endl;
    std::cout << "=======================================================" << std::endl;
    
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Initial position verification: " << (verify_starting_position(pos) ? "PASS" : "FAIL") << std::endl;
    
    // Test each problematic move with single make/unmake
    std::vector<std::string> problem_moves = {"h2h3", "c2c4", "g2g4", "b1c3"};
    
    for (const std::string& target_move : problem_moves) {
        std::cout << "\nTesting single make/unmake for " << target_move << ":" << std::endl;
        
        // Find the move
        BitboardMoveList moves;
        generate_all_moves(pos, moves);
        
        for (const auto& move : moves.moves) {
            SimpleBitboardMove simple_move = convert_move(move);
            std::string move_str = move_to_string(simple_move);
            
            if (move_str == target_move) {
                if (pos.is_legal_move(simple_move)) {
                    std::cout << "  Before make_move: " << (verify_starting_position(pos) ? "PASS" : "FAIL") << std::endl;
                    
                    // Make the move
                    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
                    
                    // Unmake the move immediately
                    pos.unmake_move(simple_move, undo_info);
                    
                    std::cout << "  After unmake_move: " << (verify_starting_position(pos) ? "PASS" : "FAIL") << std::endl;
                    
                    // Now test perft after this single cycle
                    uint64_t perft_result = perft_legal(pos, 2);
                    std::cout << "  Perft(2) for " << target_move << " after cycle: " << perft_result << std::endl;
                    
                } else {
                    std::cout << "  " << target_move << " is not legal!" << std::endl;
                }
                break;
            }
        }
    }
    
    std::cout << "\nFinal position verification: " << (verify_starting_position(pos) ? "PASS" : "FAIL") << std::endl;
    
    return 0;
}