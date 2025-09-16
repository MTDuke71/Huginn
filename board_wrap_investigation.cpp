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
    result += char('a' + (move.from_64 % 8));
    result += char('1' + (move.from_64 / 8));
    result += char('a' + (move.to_64 % 8));
    result += char('1' + (move.to_64 / 8));
    return result;
}

// Square to algebraic
std::string square_to_alg(int square) {
    if (square == -1) return "-";
    return std::string(1, 'a' + (square % 8)) + std::string(1, '1' + (square / 8));
}

int main() {
    std::cout << "=== Board Wrap Bug Investigation ===\n\n";
    
    // Recreate the exact sequence: a2a3 a7a5 h2h4
    BitboardPosition pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // a2a3
    SimpleBitboardMove a2a3;
    a2a3.from_64 = 8; a2a3.to_64 = 16;
    a2a3.is_capture = false; a2a3.is_ep_capture = false; a2a3.is_castling = false; a2a3.is_promotion = false;
    pos.make_move_with_undo(a2a3);
    
    // a7a5
    SimpleBitboardMove a7a5;
    a7a5.from_64 = 48; a7a5.to_64 = 32;
    a7a5.is_capture = false; a7a5.is_ep_capture = false; a7a5.is_castling = false; a7a5.is_promotion = false;
    pos.make_move_with_undo(a7a5);
    
    // h2h4
    SimpleBitboardMove h2h4;
    h2h4.from_64 = 15; h2h4.to_64 = 31;
    h2h4.is_capture = false; h2h4.is_ep_capture = false; h2h4.is_castling = false; h2h4.is_promotion = false;
    pos.make_move_with_undo(h2h4);
    
    std::cout << "After a2a3 a7a5 h2h4:\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    std::cout << "En passant square: " << square_to_alg(pos.ep_square_64) << "\n\n";
    
    // Expected: En passant square should be h3 (square 23) due to h2h4
    // But what if it's still a6 due to a bug?
    
    std::cout << "=== Analyzing All Generated Moves ===\n\n";
    
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    int total_moves = 0;
    int en_passant_moves = 0;
    
    std::cout << "All legal moves:\n";
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        if (pos.is_legal_move(simple_move)) {
            std::string move_str = move_to_string(simple_move);
            std::cout << move_str;
            
            if (simple_move.is_ep_capture) {
                std::cout << " (EN PASSANT!)";
                en_passant_moves++;
            }
            std::cout << "\n";
            total_moves++;
        }
    }
    
    std::cout << "\nTotal legal moves: " << total_moves << "\n";
    std::cout << "En passant moves: " << en_passant_moves << "\n";
    std::cout << "Expected total: 21\n";
    
    if (total_moves == 22) {
        std::cout << "\n🎯 CONFIRMED: We have the +1 over-generation!\n";
        if (en_passant_moves > 0) {
            std::cout << "❌ Invalid en passant move(s) detected!\n";
        }
    }
    
    // Debug en passant logic specifically
    std::cout << "\n=== En Passant Debug ===\n";
    std::cout << "Current en passant square: " << pos.ep_square_64 << " (" << square_to_alg(pos.ep_square_64) << ")\n";
    
    if (pos.ep_square_64 != -1) {
        int ep_square = pos.ep_square_64;
        std::cout << "En passant square file: " << (ep_square % 8) << "\n";
        std::cout << "En passant square rank: " << (ep_square / 8) << "\n";
        
        if (ep_square >= 16 && ep_square <= 23) { // rank 3 for black en passant
            std::cout << "This is a rank 3 en passant (for black pawns)\n";
            
            // Check what our en passant logic would do
            if ((ep_square % 8) < 7) {
                int left_pawn_square = ep_square + 7;
                std::cout << "Checking for black pawn at " << square_to_alg(left_pawn_square) << ": ";
                if (pos.piece_at(left_pawn_square) == Piece::BlackPawn) {
                    std::cout << "FOUND! (would generate en passant)\n";
                } else {
                    std::cout << "Not found\n";
                }
            }
            
            if ((ep_square % 8) > 0) {
                int right_pawn_square = ep_square + 9;
                std::cout << "Checking for black pawn at " << square_to_alg(right_pawn_square) << ": ";
                if (pos.piece_at(right_pawn_square) == Piece::BlackPawn) {
                    std::cout << "FOUND! (would generate en passant)\n";
                } else {
                    std::cout << "Not found\n";
                }
            }
        } else {
            std::cout << "En passant square is not on rank 3\n";
        }
    }
    
    return 0;
}