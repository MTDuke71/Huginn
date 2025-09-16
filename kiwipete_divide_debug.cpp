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

std::string move_to_string(const SimpleBitboardMove& move) {
    char from_file = 'a' + (move.from_64 % 8);
    char from_rank = '1' + (move.from_64 / 8);
    char to_file = 'a' + (move.to_64 % 8);
    char to_rank = '1' + (move.to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

// Perft divide function
uint64_t perft_divide(BitboardPosition& pos, int depth, const std::string& move_prefix = "") {
    if (depth == 0) return 1;
    
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    uint64_t total_nodes = 0;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        
        // Test with copy to see if move is legal
        BitboardPosition copy = pos;
        if (copy.make_move(simple_move)) {
            // Move is legal, now make it on our position and recurse
            BitboardPosition::UndoInfo undo = pos.make_move_with_undo(simple_move);
            uint64_t nodes = perft_divide(pos, depth - 1);
            pos.unmake_move(simple_move, undo);
            
            if (depth == 2) {  // Only print for depth 2 divide
                std::cout << move_prefix << move_str << " - " << nodes << "\n";
            }
            
            total_nodes += nodes;
        }
    }
    
    return total_nodes;
}

int main() {
    std::cout << "=== Analyze After e2b5 Move ===\n\n";
    
    // Kiwipete position FEN
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Initial position: " << fen << "\n\n";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }
    
    // Make e2b5 move (e2 = square 12, b5 = square 33)
    SimpleBitboardMove e2b5_move;
    e2b5_move.from_64 = 12;  // e2
    e2b5_move.to_64 = 33;    // b5
    e2b5_move.is_capture = true;  // captures knight on b5
    
    BitboardPosition::UndoInfo undo = pos.make_move_with_undo(e2b5_move);
    
    std::cout << "After e2b5:\n";
    std::cout << "New FEN: " << pos.to_fen() << "\n\n";
    
    // Now analyze Black's moves
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    std::cout << "Black's moves (" << moves.size() << " total):\n";
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        std::string move_str = move_to_string(simple_move);
        std::cout << "  " << move_str;
        if (simple_move.is_castling) std::cout << " (castling)";
        if (simple_move.is_capture) std::cout << " (capture)";
        std::cout << "\n";
    }
    
    // Check specific problematic moves
    std::cout << "\n=== Attack Analysis ===\n";
    int black_king = pos.king_square_64[1];  // Black king
    std::cout << "Black king on square " << black_king << " (e8)\n";
    
    // Check if e8 is attacked by White pieces
    std::cout << "Is e8 (square 60) attacked by White? ";
    bool e8_attacked = pos.is_square_attacked(60, Color::White);
    std::cout << (e8_attacked ? "YES" : "NO") << "\n";
    
    // Check castling rights
    std::cout << "Castling rights: " << pos.castling_rights << "\n";
    std::cout << "Black kingside available: " << ((pos.castling_rights & 4) ? "YES" : "NO") << "\n";
    std::cout << "Black queenside available: " << ((pos.castling_rights & 8) ? "YES" : "NO") << "\n";
    
    // Check if specific squares are attacked
    std::cout << "Is c8 (square 58) attacked by White? " << (pos.is_square_attacked(58, Color::White) ? "YES" : "NO") << "\n";
    std::cout << "Is d8 (square 59) attacked by White? " << (pos.is_square_attacked(59, Color::White) ? "YES" : "NO") << "\n";
    std::cout << "Is f8 (square 61) attacked by White? " << (pos.is_square_attacked(61, Color::White) ? "YES" : "NO") << "\n";
    std::cout << "Is g8 (square 62) attacked by White? " << (pos.is_square_attacked(62, Color::White) ? "YES" : "NO") << "\n";
    
    // Check specifically for the bishop on b5
    std::cout << "\nWhite bishop on b5 (square 33) should attack e8...\n";
    
    return 0;
}