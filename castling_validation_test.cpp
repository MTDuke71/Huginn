#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;

std::string move_to_string(int from_64, int to_64) {
    char from_file = 'a' + (from_64 % 8);
    char from_rank = '1' + (from_64 / 8);
    char to_file = 'a' + (to_64 % 8);
    char to_rank = '1' + (to_64 / 8);
    return std::string(1, from_file) + from_rank + to_file + to_rank;
}

int main() {
    std::cout << "=== Castling Move Validation Test ===\n\n";
    
    // Kiwipete position FEN
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::cout << "Testing position: " << fen << "\n\n";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN string!\n";
        return 1;
    }
    
    std::cout << "Castling rights: " << static_cast<int>(pos.castling_rights) << "\n";
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << "\n\n";
    
    // Test specific castling moves
    SimpleBitboardMove kingside_castle;
    kingside_castle.from_64 = 4;  // e1
    kingside_castle.to_64 = 6;    // g1
    kingside_castle.is_castling = true;
    kingside_castle.is_capture = false;
    kingside_castle.is_ep_capture = false;
    kingside_castle.is_promotion = false;
    
    SimpleBitboardMove queenside_castle;
    queenside_castle.from_64 = 4;  // e1
    queenside_castle.to_64 = 2;    // c1
    queenside_castle.is_castling = true;
    queenside_castle.is_capture = false;
    queenside_castle.is_ep_capture = false;
    queenside_castle.is_promotion = false;
    
    // Test kingside castling
    BitboardPosition test_pos = pos;
    bool kingside_legal = test_pos.make_move(kingside_castle);
    std::cout << "Kingside castling (e1g1): " << (kingside_legal ? "LEGAL" : "ILLEGAL") << "\n";
    
    // Test queenside castling  
    test_pos = pos;
    bool queenside_legal = test_pos.make_move(queenside_castle);
    std::cout << "Queenside castling (e1c1): " << (queenside_legal ? "LEGAL" : "ILLEGAL") << "\n";
    
    // Check what our move generator produces
    std::cout << "\n=== Generated Castling Moves ===\n";
    BitboardMoveList moves;
    generate_all_moves(pos, moves);
    
    int castling_count = 0;
    for (const auto& move : moves.moves) {
        if (move.is_castling) {
            std::cout << "Generated: " << move_to_string(move.from_64, move.to_64) 
                      << " (castling)" << "\n";
            castling_count++;
        }
    }
    
    std::cout << "\nTotal castling moves generated: " << castling_count << "\n";
    std::cout << "Expected: 2 (if both are legal)\n";
    
    return 0;
}