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

// Perft WITHOUT legal checking
uint64_t perft_no_legal(BitboardPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    uint64_t nodes = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(simple_move);
        nodes += perft_no_legal(pos, depth - 1);
        pos.unmake_move(simple_move, undo_info);
    }
    
    return nodes;
}

int main() {
    std::cout << "Testing h2h3 from starting position with perft(2)" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Create h2h3 move
    SimpleBitboardMove h2h3;
    h2h3.from_64 = 15;  // h2
    h2h3.to_64 = 23;    // h3
    h2h3.is_capture = false;
    h2h3.is_ep_capture = false;
    h2h3.is_castling = false;
    h2h3.is_promotion = false;
    h2h3.promotion_type = PieceType::None;
    
    std::cout << "Initial position verification:" << std::endl;
    std::cout << "  Side to move: " << (pos.get_side_to_move_const() == Color::White ? "White" : "Black") << std::endl;
    std::cout << "  Is h2h3 legal: " << (pos.is_legal_move(h2h3) ? "Yes" : "No") << std::endl;
    
    std::cout << "\nMaking h2h3..." << std::endl;
    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(h2h3);
    
    std::cout << "After h2h3:" << std::endl;
    std::cout << "  Side to move: " << (pos.get_side_to_move_const() == Color::White ? "White" : "Black") << std::endl;
    
    // Count legal moves in the resulting position
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    int legal_count = 0;
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            legal_count++;
        }
    }
    std::cout << "  Legal moves available: " << legal_count << std::endl;
    
    // Run perft(1) and perft(2) from this position
    std::cout << "\nPerft results from position after h2h3:" << std::endl;
    uint64_t perft1_with_legal = perft_legal(pos, 1);
    std::cout << "  Perft(1) with legal check: " << perft1_with_legal << std::endl;
    
    uint64_t perft2_with_legal = perft_legal(pos, 2);
    std::cout << "  Perft(2) with legal check: " << perft2_with_legal << std::endl;
    
    uint64_t perft1_no_legal = perft_no_legal(pos, 1);
    std::cout << "  Perft(1) without legal check: " << perft1_no_legal << std::endl;
    
    uint64_t perft2_no_legal = perft_no_legal(pos, 2);
    std::cout << "  Perft(2) without legal check: " << perft2_no_legal << std::endl;
    
    // Unmake the move
    std::cout << "\nUnmaking h2h3..." << std::endl;
    pos.unmake_move(h2h3, undo_info);
    
    std::cout << "After unmake:" << std::endl;
    std::cout << "  Side to move: " << (pos.get_side_to_move_const() == Color::White ? "White" : "Black") << std::endl;
    std::cout << "  Is h2h3 still legal: " << (pos.is_legal_move(h2h3) ? "Yes" : "No") << std::endl;
    
    // Test the exact divide function logic
    std::cout << "\nTesting exact divide function logic for h2h3:" << std::endl;
    if (pos.is_legal_move(h2h3)) {
        BitboardPosition::UndoInfo undo_info2 = pos.make_move_with_undo(h2h3);
        uint64_t nodes = (3 <= 1) ? 1 : perft_legal(pos, 2);  // This is the exact logic from divide function
        pos.unmake_move(h2h3, undo_info2);
        std::cout << "  Divide function result: " << nodes << std::endl;
    }
    
    return 0;
}