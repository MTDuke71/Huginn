#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_attacks.hpp"
#include <iostream>

int main() {
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        std::cout << "ERROR: Failed to parse FEN\n";
        return 1;
    }
    
    // Test 1: Count moves before any operations
    BitboardMoveList moves1;
    BitboardMoveGen::generate_legal_moves(pos, moves1);
    std::cout << "Before f3f5: " << moves1.moves.size() << " moves\n";
    
    // Test 2: Make f3f5 move
    SimpleBitboardMove f3f5_move;
    f3f5_move.from_64 = 21;  // f3
    f3f5_move.to_64 = 37;    // f5
    f3f5_move.is_capture = false;
    f3f5_move.is_ep_capture = false;
    f3f5_move.is_castling = false;
    f3f5_move.is_promotion = false;
    
    BitboardPosition::UndoInfo undo_info = pos.make_move_with_undo(f3f5_move);
    
    // Test 3: Count moves after f3f5
    BitboardMoveList moves2;
    BitboardMoveGen::generate_legal_moves(pos, moves2);
    std::cout << "After f3f5: " << moves2.moves.size() << " moves\n";
    
    // Test 4: Unmake f3f5 move
    pos.unmake_move(f3f5_move, undo_info);
    
    // Test 5: Count moves after unmake
    BitboardMoveList moves3;
    BitboardMoveGen::generate_legal_moves(pos, moves3);
    std::cout << "After unmake f3f5: " << moves3.moves.size() << " moves\n";
    
    // Test 6: Check if position is identical to original
    std::string original_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    std::string restored_fen = pos.to_fen();
    
    std::cout << "\nOriginal FEN: " << original_fen << "\n";
    std::cout << "Restored FEN: " << restored_fen << "\n";
    std::cout << "FEN Match: " << (original_fen == restored_fen ? "YES" : "NO") << "\n";
    
    if (moves1.moves.size() != moves3.moves.size()) {
        std::cout << "\n❌ BUG FOUND: Move count changed after make/unmake!\n";
        std::cout << "Expected: " << moves1.moves.size() << " moves\n";
        std::cout << "Got: " << moves3.moves.size() << " moves\n";
    } else {
        std::cout << "\n✅ Move count preserved after make/unmake\n";
    }
    
    return 0;
}