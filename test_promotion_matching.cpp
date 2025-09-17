#include "src/position.hpp"
#include "src/movegen_enhanced.hpp"
#include "src/init.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>

using namespace BitboardMoveGen;

// Convert BitboardMoveList::BitboardMove to SimpleBitboardMove
SimpleBitboardMove convert_move(const BitboardMoveList::BitboardMove& move) {
    SimpleBitboardMove simple_move;
    simple_move.from_64 = move.from_64;
    simple_move.to_64 = move.to_64;
    simple_move.promotion_type = move.promotion_type;
    simple_move.is_capture = move.is_capture;
    simple_move.is_ep_capture = move.is_ep_capture;
    simple_move.is_castling = move.is_castling;
    simple_move.is_promotion = move.promotion_type != PieceType::None;
    return simple_move;
}

// Convert 120-square to 64-square
int square120_to_64(int sq120) {
    int rank = sq120 / 10 - 2;
    int file = sq120 % 10 - 1;
    if (rank < 0 || rank > 7 || file < 0 || file > 7) return -1;
    return rank * 8 + file;
}

// Convert 64-square to algebraic notation
std::string sq64_to_alg(int sq64) {
    if (sq64 < 0 || sq64 > 63) return "??";
    char file = 'a' + (sq64 % 8);
    char rank = '1' + (sq64 / 8);
    return std::string(1, file) + std::string(1, rank);
}

std::string piece_type_to_string(PieceType pt) {
    switch(pt) {
        case PieceType::Queen: return "Q";
        case PieceType::Rook: return "R";
        case PieceType::Bishop: return "B";
        case PieceType::Knight: return "N";
        case PieceType::None: return "";
        default: return "?";
    }
}

void test_promotion_move_matching() {
    std::cout << "=== Test Promotion Move Matching ===\n\n";

    // Position after b2b3
    std::string after_b2b3_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/PPN2Q2/2PBBPpP/R3K2R b KQkq - 0 2";
    
    Position vice_pos;
    vice_pos.set_from_fen(after_b2b3_fen);
    
    BitboardPosition bb_pos;
    bb_pos.set_from_fen(after_b2b3_fen);
    
    std::cout << "Position: " << after_b2b3_fen << "\n\n";
    
    // Get moves
    S_MOVELIST vice_moves;
    generate_all_moves(vice_pos, vice_moves);
    
    BitboardMoveList bb_moves;
    generate_legal_moves(bb_pos, bb_moves);
    
    std::cout << "=== BitboardPosition g2 Promotion Moves in Detail ===\n";
    for (const auto& move : bb_moves.moves) {
        if (move.from_64 == 14) { // g2
            std::cout << "Move: " << sq64_to_alg(move.from_64) << sq64_to_alg(move.to_64);
            std::cout << " promote_to=" << piece_type_to_string(move.promotion_type);
            std::cout << " is_promotion=" << (move.is_promotion ? "true" : "false");
            std::cout << " is_capture=" << (move.is_capture ? "true" : "false");
            std::cout << "\n";
            
            // Test making this specific move
            BitboardPosition test_pos = bb_pos;
            SimpleBitboardMove simple_move = convert_move(move);
            
            std::cout << "  Before move: " << test_pos.to_fen() << "\n";
            BitboardPosition::UndoInfo undo = test_pos.make_move_with_undo(simple_move);
            std::cout << "  After move:  " << test_pos.to_fen() << "\n";
            
            // Check what piece is actually on the destination
            Piece piece_on_dest = test_pos.piece_at(move.to_64);
            PieceType actual_type = type_of(piece_on_dest);
            std::cout << "  Actual piece on " << sq64_to_alg(move.to_64) << ": " << piece_type_to_string(actual_type);
            std::cout << " (expected: " << piece_type_to_string(move.promotion_type) << ")";
            
            if (actual_type == move.promotion_type) {
                std::cout << " ✓\n";
            } else {
                std::cout << " ❌ MISMATCH!\n";
            }
            
            test_pos.unmake_move(simple_move, undo);
            std::cout << "  After undo:  " << test_pos.to_fen() << "\n";
            std::cout << "\n";
        }
    }
}

int main() {
    Huginn::init();
    
    test_promotion_move_matching();
    
    return 0;
}