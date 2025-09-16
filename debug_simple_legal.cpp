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
    result += char('a' + file_of_64(move.from_64));
    result += char('1' + rank_of_64(move.from_64));
    result += char('a' + file_of_64(move.to_64));
    result += char('1' + rank_of_64(move.to_64));
    return result;
}

int main() {
    std::cout << "Testing basic move legality with side_to_move fix" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Initial side_to_move: " << (int)pos.side_to_move << " (0=WHITE, 1=BLACK)" << std::endl;
    
    // Test a2a3 move
    SimpleBitboardMove a2a3;
    a2a3.from_64 = 8;  // a2
    a2a3.to_64 = 16;   // a3
    a2a3.is_capture = false;
    a2a3.is_ep_capture = false;
    a2a3.is_castling = false;
    a2a3.is_promotion = false;
    a2a3.promotion_type = PieceType::None;
    
    std::cout << "Testing a2a3..." << std::endl;
    std::cout << "Before is_legal_move: side_to_move = " << (int)pos.side_to_move << std::endl;
    
    bool is_legal = pos.is_legal_move(a2a3);
    
    std::cout << "After is_legal_move: side_to_move = " << (int)pos.side_to_move << std::endl;
    std::cout << "a2a3 is legal: " << (is_legal ? "YES" : "NO") << std::endl;
    
    // Test again to see if it's still consistent
    std::cout << "\nTesting a2a3 again..." << std::endl;
    std::cout << "Before 2nd is_legal_move: side_to_move = " << (int)pos.side_to_move << std::endl;
    
    bool is_legal2 = pos.is_legal_move(a2a3);
    
    std::cout << "After 2nd is_legal_move: side_to_move = " << (int)pos.side_to_move << std::endl;
    std::cout << "a2a3 is legal (2nd test): " << (is_legal2 ? "YES" : "NO") << std::endl;
    
    return 0;
}