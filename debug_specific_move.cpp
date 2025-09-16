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

int main() {
    // Set up starting position
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    std::cout << "Testing specific moves that give 0 in divide:" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // Test h2h3 (move 8 in processing order)
    std::cout << "\nTesting h2h3:" << std::endl;
    SimpleBitboardMove h2h3;
    h2h3.from_64 = 15;  // h2
    h2h3.to_64 = 23;    // h3
    h2h3.is_capture = false;
    h2h3.is_ep_capture = false;
    h2h3.is_castling = false;
    h2h3.is_promotion = false;
    h2h3.promotion_type = PieceType::None;
    
    if (pos.is_legal_move(h2h3)) {
        std::cout << "h2h3 is legal" << std::endl;
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(h2h3);
        uint64_t nodes = perft_legal(pos, 2);
        std::cout << "h2h3 perft(2): " << nodes << std::endl;
        pos.unmake_move(h2h3, undo_info);
        std::cout << "After unmake, position should be starting position" << std::endl;
    } else {
        std::cout << "h2h3 is NOT legal!" << std::endl;
    }
    
    // Test c2c4 (move 11 in processing order)
    std::cout << "\nTesting c2c4:" << std::endl;
    SimpleBitboardMove c2c4;
    c2c4.from_64 = 10;  // c2
    c2c4.to_64 = 26;    // c4
    c2c4.is_capture = false;
    c2c4.is_ep_capture = false;
    c2c4.is_castling = false;
    c2c4.is_promotion = false;
    c2c4.promotion_type = PieceType::None;
    
    if (pos.is_legal_move(c2c4)) {
        std::cout << "c2c4 is legal" << std::endl;
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(c2c4);
        uint64_t nodes = perft_legal(pos, 2);
        std::cout << "c2c4 perft(2): " << nodes << std::endl;
        pos.unmake_move(c2c4, undo_info);
        std::cout << "After unmake, position should be starting position" << std::endl;
    } else {
        std::cout << "c2c4 is NOT legal!" << std::endl;
    }

    // Test g2g4 (move 15 in processing order)
    std::cout << "\nTesting g2g4:" << std::endl;
    SimpleBitboardMove g2g4;
    g2g4.from_64 = 14;  // g2
    g2g4.to_64 = 30;    // g4
    g2g4.is_capture = false;
    g2g4.is_ep_capture = false;
    g2g4.is_castling = false;
    g2g4.is_promotion = false;
    g2g4.promotion_type = PieceType::None;
    
    if (pos.is_legal_move(g2g4)) {
        std::cout << "g2g4 is legal" << std::endl;
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(g2g4);
        uint64_t nodes = perft_legal(pos, 2);
        std::cout << "g2g4 perft(2): " << nodes << std::endl;
        pos.unmake_move(g2g4, undo_info);
        std::cout << "After unmake, position should be starting position" << std::endl;
    } else {
        std::cout << "g2g4 is NOT legal!" << std::endl;
    }

    // Test b1c3 (move 18 in processing order)
    std::cout << "\nTesting b1c3:" << std::endl;
    SimpleBitboardMove b1c3;
    b1c3.from_64 = 1;   // b1
    b1c3.to_64 = 18;    // c3
    b1c3.is_capture = false;
    b1c3.is_ep_capture = false;
    b1c3.is_castling = false;
    b1c3.is_promotion = false;
    b1c3.promotion_type = PieceType::None;
    
    if (pos.is_legal_move(b1c3)) {
        std::cout << "b1c3 is legal" << std::endl;
        BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(b1c3);
        uint64_t nodes = perft_legal(pos, 2);
        std::cout << "b1c3 perft(2): " << nodes << std::endl;
        pos.unmake_move(b1c3, undo_info);
        std::cout << "After unmake, position should be starting position" << std::endl;
    } else {
        std::cout << "b1c3 is NOT legal!" << std::endl;
    }
    
    return 0;
}