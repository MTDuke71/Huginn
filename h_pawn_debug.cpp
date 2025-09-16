#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;

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
    std::cout << "=== Debug H-Pawn Moves ===\n\n";
    
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Test h2h3 specifically
    std::cout << "Testing h2h3 (h2=15, h3=23):\n";
    SimpleBitboardMove h2h3;
    h2h3.from_64 = 15;  // h2
    h2h3.to_64 = 23;    // h3
    h2h3.is_capture = false;
    h2h3.is_ep_capture = false;
    h2h3.is_castling = false;
    h2h3.is_promotion = false;
    h2h3.promotion_type = PieceType::None;
    
    if (pos.is_legal_move(h2h3)) {
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(h2h3);
        std::cout << "Position after h2h3: " << pos.to_fen() << "\n";
        uint64_t nodes = perft_legal(pos, 2);
        std::cout << "Perft 2 result: " << nodes << " (expected: 380)\n";
        pos.unmake_move(h2h3, undo);
        std::cout << "Position restored: " << pos.to_fen() << "\n";
    } else {
        std::cout << "h2h3 is not legal!\n";
    }
    
    std::cout << "\n";
    
    // Test h2h4 specifically
    std::cout << "Testing h2h4 (h2=15, h4=31):\n";
    SimpleBitboardMove h2h4;
    h2h4.from_64 = 15;  // h2
    h2h4.to_64 = 31;    // h4
    h2h4.is_capture = false;
    h2h4.is_ep_capture = false;
    h2h4.is_castling = false;
    h2h4.is_promotion = false;
    h2h4.promotion_type = PieceType::None;
    
    if (pos.is_legal_move(h2h4)) {
        BitboardPosition::UndoInfo undo = pos.make_move_with_undo(h2h4);
        std::cout << "Position after h2h4: " << pos.to_fen() << "\n";
        uint64_t nodes = perft_legal(pos, 2);
        std::cout << "Perft 2 result: " << nodes << " (expected: 421)\n";
        pos.unmake_move(h2h4, undo);
        std::cout << "Position restored: " << pos.to_fen() << "\n";
    } else {
        std::cout << "h2h4 is not legal!\n";
    }
    
    return 0;
}