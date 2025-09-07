#include "src/init.hpp"
#include "src/minimal_search.hpp"
#include "src/position.hpp"
#include <iostream>

using namespace Huginn;

int main() {
    init();
    
    std::cout << "=== Debug Null Move Conditions ===" << std::endl;
    
    // Test position: A middlegame position with many pieces
    std::string fen = "r1bq1rk1/pp2nppp/2n1p3/3pP3/2PP4/2N1BN2/PP3PPP/R2QK2R w KQ - 0 10";
    Position pos;
    pos.set_from_fen(fen);
    
    std::cout << "Position: " << fen << std::endl;
    std::cout << "Side to move: " << (pos.side_to_move == Color::White ? "White" : "Black") << std::endl;
    
    // Check null move conditions manually
    bool in_check = SqAttacked(pos.king_sq[int(pos.side_to_move)], pos, !pos.side_to_move);
    bool has_pieces = pos.has_non_pawn_material(pos.side_to_move);
    
    std::cout << "In check: " << (in_check ? "YES" : "NO") << std::endl;
    std::cout << "Has non-pawn material: " << (has_pieces ? "YES" : "NO") << std::endl;
    std::cout << "King square: " << pos.king_sq[int(pos.side_to_move)] << std::endl;
    
    // Show piece counts for current side
    int color_idx = int(pos.side_to_move);
    std::cout << "Piece counts for current side:" << std::endl;
    std::cout << "  Queens: " << pos.pCount[color_idx][int(PieceType::Queen)] << std::endl;
    std::cout << "  Rooks: " << pos.pCount[color_idx][int(PieceType::Rook)] << std::endl;
    std::cout << "  Bishops: " << pos.pCount[color_idx][int(PieceType::Bishop)] << std::endl;
    std::cout << "  Knights: " << pos.pCount[color_idx][int(PieceType::Knight)] << std::endl;
    std::cout << "  Pawns: " << pos.pCount[color_idx][int(PieceType::Pawn)] << std::endl;
    
    std::cout << std::endl;
    std::cout << "For null move to trigger we need:" << std::endl;
    std::cout << "- doNull = true (we control this)" << std::endl;
    std::cout << "- Not in check: " << (!in_check ? "✓" : "✗") << std::endl;
    std::cout << "- Not at root (we control this)" << std::endl;
    std::cout << "- Depth >= 4 (we control this)" << std::endl;
    std::cout << "- Has big pieces: " << (has_pieces ? "✓" : "✗") << std::endl;
    
    return 0;
}
