#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

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

int main() {
    std::cout << "=== Position Analysis After e5c6 ===\n\n";
    
    // Kiwipete position FEN
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Starting position: " << fen << "\n";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }
    
    // Make the move e5c6
    SimpleBitboardMove e5c6;
    e5c6.from_64 = 36; // e5
    e5c6.to_64 = 42;   // c6
    e5c6.is_capture = true; // This captures the black knight on c6
    e5c6.is_castling = false;
    e5c6.is_ep_capture = false;
    e5c6.is_promotion = false;
    
    std::cout << "Making move: e5c6\n\n";
    
    if (!pos.make_move(e5c6)) {
        std::cout << "ERROR: Failed to make move e5c6!\n";
        return 1;
    }
    
    std::cout << "Position after e5c6:\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n";
    std::cout << "Castling rights: " << static_cast<int>(pos.castling_rights) << "\n";
    std::cout << "FEN: " << pos.to_fen() << "\n\n";
    
    // Generate all moves in the new position
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    std::vector<std::string> our_moves;
    std::vector<std::string> castling_moves;
    
    for (const auto& move : moves.moves) {
        SimpleBitboardMove simple_move = convert_move(move);
        
        // Test if move is legal
        BitboardPosition copy = pos;
        if (copy.make_move(simple_move)) {
            std::string move_str = move_to_string(simple_move);
            our_moves.push_back(move_str);
            
            if (move.is_castling) {
                castling_moves.push_back(move_str);
            }
        }
    }
    
    std::sort(our_moves.begin(), our_moves.end());
    
    std::cout << "Our moves (" << our_moves.size() << "):\n";
    for (const auto& move : our_moves) {
        std::cout << move << " - 1\n";
    }
    
    if (!castling_moves.empty()) {
        std::cout << "\nCASTLING MOVES FOUND:\n";
        for (const auto& move : castling_moves) {
            std::cout << "  " << move << " (SHOULD THIS BE LEGAL?)\n";
        }
    }
    
    std::cout << "\nTotal: " << our_moves.size() << "\n";
    std::cout << "Expected: 41\n";
    std::cout << "Difference: " << (static_cast<int>(our_moves.size()) - 41) << "\n";
    
    return 0;
}