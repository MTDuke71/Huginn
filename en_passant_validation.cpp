#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

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
    result += char('a' + (move.from_64 % 8));
    result += char('1' + (move.from_64 / 8));
    result += char('a' + (move.to_64 % 8));
    result += char('1' + (move.to_64 / 8));
    return result;
}

int main() {
    std::cout << "=== En Passant Validation Test ===\n\n";
    
    // Test starting position for any improper en passant generation
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing starting position (should have NO en passant moves):\n";
    std::cout << "En passant square: " << pos.ep_square_64 << " (should be -1)\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    int en_passant_count = 0;
    std::cout << "En passant moves found:\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            if (simple_move.is_ep_capture) {
                std::cout << "❌ " << move_to_string(simple_move) << " (EN PASSANT - should NOT exist!)\n";
                en_passant_count++;
            }
        }
    }
    
    if (en_passant_count == 0) {
        std::cout << "✅ No invalid en passant moves found in starting position\n";
    } else {
        std::cout << "❌ Found " << en_passant_count << " invalid en passant moves!\n";
    }
    
    std::cout << "\n" << "=== Testing En Passant Position ===\n";
    
    // Test a position where en passant should be available
    pos.set_from_fen("rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
    std::cout << "Position after 1...d5 (white can capture en passant):\n";
    std::cout << "En passant square: " << pos.ep_square_64 << " (should be d6 = square 43)\n\n";
    
    generate_all_moves(pos, moves);
    
    en_passant_count = 0;
    std::cout << "En passant moves found:\n";
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            if (simple_move.is_ep_capture) {
                std::cout << "✅ " << move_to_string(simple_move) << " (EN PASSANT - should exist!)\n";
                en_passant_count++;
            }
        }
    }
    
    if (en_passant_count == 1) {
        std::cout << "✅ Correct number of en passant moves found\n";
    } else {
        std::cout << "❌ Expected 1 en passant move, found " << en_passant_count << "\n";
    }
    
    return 0;
}