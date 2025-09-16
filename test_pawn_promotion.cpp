#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include <iostream>

using namespace BitboardMoveGen;

int main() {
    std::cout << "=== Pawn Promotion Test ===\n\n";
    
    // Test position with white pawn about to promote
    std::string test_fen = "8/P7/8/8/8/8/8/8 w - - 0 1";  // White pawn on a7 about to promote
    std::cout << "Test position: " << test_fen << "\n";
    
    BitboardPosition pos;
    if (!pos.set_from_fen(test_fen)) {
        std::cout << "ERROR: Failed to parse FEN!\n";
        return 1;
    }
    
    BitboardMoveList moves;
    generate_legal_moves(pos, moves);
    
    std::cout << "Generated moves (" << moves.moves.size() << " total):\n";
    
    for (const auto& move : moves.moves) {
        char from_file = 'a' + (move.from_64 % 8);
        char from_rank = '1' + (move.from_64 / 8);
        char to_file = 'a' + (move.to_64 % 8);
        char to_rank = '1' + (move.to_64 / 8);
        
        std::cout << "  " << from_file << from_rank << to_file << to_rank;
        
        if (move.is_promotion) {
            std::cout << " (promotion to ";
            switch (move.promotion_type) {
                case PieceType::Queen: std::cout << "Queen"; break;
                case PieceType::Rook: std::cout << "Rook"; break;
                case PieceType::Bishop: std::cout << "Bishop"; break;
                case PieceType::Knight: std::cout << "Knight"; break;
                default: std::cout << "Unknown"; break;
            }
            std::cout << ")";
        }
        
        std::cout << "\n";
    }
    
    std::cout << "\nExpected: 4 promotion moves (a7a8Q, a7a8R, a7a8B, a7a8N)\n";
    std::cout << "Actual: " << moves.moves.size() << " moves\n";
    
    if (moves.moves.size() == 4) {
        std::cout << "✅ CORRECT: All 4 promotion types generated\n";
    } else {
        std::cout << "❌ ERROR: Wrong number of promotion moves\n";
    }
    
    return 0;
}