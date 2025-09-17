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

void debug_promotion_moves() {
    std::cout << "=== Debug Pawn Promotion Moves g2->g1 and g2->h1 ===\n\n";

    // First, create the position after b2b3 to have the promotion bug state
    std::string after_h7g6_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/P1N2Q2/1PPBBPpP/R3K2R w KQkq - 0 2";
    
    Position vice_orig;
    vice_orig.set_from_fen(after_h7g6_fen);
    
    BitboardPosition bb_orig;
    bb_orig.set_from_fen(after_h7g6_fen);
    
    // Make the b2b3 move to get to the position where promotion moves are problematic
    S_MOVELIST vice_moves;
    generate_all_moves(vice_orig, vice_moves);
    
    Position vice_after_b2b3;
    bool vice_found = false;
    for (int i = 0; i < vice_moves.count; i++) {
        const auto& move = vice_moves.moves[i];
        int from_64 = square120_to_64(move.get_from());
        int to_64 = square120_to_64(move.get_to());
        
        if (from_64 == 9 && to_64 == 17) { // b2b3
            vice_after_b2b3 = vice_orig;
            if (vice_after_b2b3.MakeMove(move) == 1) {
                vice_found = true;
                break;
            }
        }
    }
    
    BitboardMoveList bb_moves;
    generate_legal_moves(bb_orig, bb_moves);
    
    BitboardPosition bb_after_b2b3 = bb_orig;
    bool bb_found = false;
    for (const auto& move : bb_moves.moves) {
        if (move.from_64 == 9 && move.to_64 == 17) { // b2b3
            SimpleBitboardMove simple_move = convert_move(move);
            bb_after_b2b3.make_move_with_undo(simple_move);
            bb_found = true;
            break;
        }
    }
    
    if (!vice_found || !bb_found) {
        std::cout << "❌ Could not make b2b3 move\n";
        return;
    }
    
    std::cout << "Position after b2b3: " << bb_after_b2b3.to_fen() << "\n\n";
    
    // Now examine the promotion moves
    std::cout << "=== VICE g2 Promotion Moves ===\n";
    S_MOVELIST vice_promo_moves;
    generate_all_moves(vice_after_b2b3, vice_promo_moves);
    
    int vice_g2_moves = 0;
    for (int i = 0; i < vice_promo_moves.count; i++) {
        const auto& move = vice_promo_moves.moves[i];
        int from_64 = square120_to_64(move.get_from());
        int to_64 = square120_to_64(move.get_to());
        
        if (from_64 == 14) { // g2 (square 14)
            Position test_pos = vice_after_b2b3;
            if (test_pos.MakeMove(move) == 1) {
                vice_g2_moves++;
                std::cout << "  " << sq64_to_alg(from_64) << sq64_to_alg(to_64);
                if (move.is_promotion()) {
                    // Try to get promotion piece info from the move
                    std::cout << " (promotion)";
                }
                std::cout << "\n";
                test_pos.TakeMove();
            }
        }
    }
    std::cout << "Total VICE g2 moves: " << vice_g2_moves << "\n\n";
    
    std::cout << "=== BitboardPosition g2 Promotion Moves ===\n";
    BitboardMoveList bb_promo_moves;
    generate_legal_moves(bb_after_b2b3, bb_promo_moves);
    
    int bb_g2_moves = 0;
    for (const auto& move : bb_promo_moves.moves) {
        if (move.from_64 == 14) { // g2
            bb_g2_moves++;
            std::cout << "  " << sq64_to_alg(move.from_64) << sq64_to_alg(move.to_64);
            if (move.is_promotion) {
                std::cout << " promote to " << piece_type_to_string(move.promotion_type);
            }
            if (move.is_capture) std::cout << " (capture)";
            if (move.is_ep_capture) std::cout << " (en passant)";
            std::cout << "\n";
        }
    }
    std::cout << "Total BitboardPosition g2 moves: " << bb_g2_moves << "\n\n";
    
    // Test specific moves g2g1 and g2h1
    std::cout << "=== Detailed Analysis of g2g1 and g2h1 ===\n";
    
    for (const std::string& target_move : {"g2g1", "g2h1"}) {
        int target_to = (target_move == "g2g1") ? 6 : 7;
        
        std::cout << "\n--- " << target_move << " ---\n";
        
        // Test VICE
        int vice_count = 0;
        for (int i = 0; i < vice_promo_moves.count; i++) {
            const auto& move = vice_promo_moves.moves[i];
            int from_64 = square120_to_64(move.get_from());
            int to_64 = square120_to_64(move.get_to());
            
            if (from_64 == 14 && to_64 == target_to) {
                Position test_pos = vice_after_b2b3;
                if (test_pos.MakeMove(move) == 1) {
                    vice_count++;
                    test_pos.TakeMove();
                }
            }
        }
        std::cout << "VICE found " << vice_count << " legal " << target_move << " moves\n";
        
        // Test BitboardPosition
        int bb_count = 0;
        for (const auto& move : bb_promo_moves.moves) {
            if (move.from_64 == 14 && move.to_64 == target_to) {
                bb_count++;
            }
        }
        std::cout << "BitboardPosition found " << bb_count << " legal " << target_move << " moves\n";
        std::cout << "Difference: " << (bb_count - vice_count) << "\n";
    }
}

int main() {
    Huginn::init();
    
    debug_promotion_moves();
    
    return 0;
}