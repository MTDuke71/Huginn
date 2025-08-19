#include <iostream>
#include "debug.hpp"
#include "position.hpp"
#include "init.hpp"

int main() {
    std::cout << "=== Debug Position Validation Demo ===" << std::endl;
    
    // Initialize the engine
    Huginn::init();
    
    // Example 1: Validate a correct starting position
    std::cout << "\n1. Testing CORRECT starting position:" << std::endl;
    Position correct_pos;
    correct_pos.set_startpos();
    
    std::string starting_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    bool result1 = Debug::validate_position_consistency(correct_pos, starting_fen);
    std::cout << "Result: " << (result1 ? "PASSED" : "FAILED") << std::endl;
    
    // Example 2: Create an intentionally corrupted position to demonstrate error detection
    std::cout << "\n\n2. Testing CORRUPTED position (bitboard mismatch):" << std::endl;
    Position corrupted_pos;
    corrupted_pos.set_startpos();
    
    // Deliberately corrupt the white pawns bitboard (but leave board array intact)
    // This will be caught by the bitboard consistency check
    // Note: We can't directly access pawns_bb from outside the class,
    // so let's corrupt the board array and then rebuild partially
    corrupted_pos.board[sq(File::E, Rank::R2)] = Piece::None;  // Remove e2 pawn from board
    // But DON'T call rebuild_counts(), so bitboards will be inconsistent
    
    std::cout << "Deliberately corrupted e2 pawn (removed from board but not from bitboards):" << std::endl;
    
    // This should catch the inconsistency
    bool result2 = Debug::validate_position_consistency(corrupted_pos, starting_fen);
    std::cout << "Result: " << (result2 ? "PASSED" : "FAILED") << std::endl;
    
    // Example 3: Validate position after some moves
    std::cout << "\n\n3. Testing position after legal moves:" << std::endl;
    Position moved_pos;
    moved_pos.set_startpos();
    
    // Make the moves: 1.e4 e5 2.Nf3 Nc6
    S_MOVE e2e4 = make_move(sq(File::E, Rank::R2), sq(File::E, Rank::R4));
    moved_pos.make_move_with_undo(e2e4);
    
    S_MOVE e7e5 = make_move(sq(File::E, Rank::R7), sq(File::E, Rank::R5));
    moved_pos.make_move_with_undo(e7e5);
    
    S_MOVE g1f3 = make_move(sq(File::G, Rank::R1), sq(File::F, Rank::R3));
    moved_pos.make_move_with_undo(g1f3);
    
    S_MOVE b8c6 = make_move(sq(File::B, Rank::R8), sq(File::C, Rank::R6));
    moved_pos.make_move_with_undo(b8c6);
    
    std::string expected_fen_after_moves = "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3";
    
    bool result3 = Debug::validate_position_consistency(moved_pos, expected_fen_after_moves);
    std::cout << "Result: " << (result3 ? "PASSED" : "FAILED") << std::endl;
    
    // Example 4: Test individual validation functions
    std::cout << "\n\n4. Testing individual validation components on starting position:" << std::endl;
    Position test_pos;
    test_pos.set_startpos();
    
    std::cout << "Individual component tests:" << std::endl;
    std::cout << "  Bitboards:      " << (Debug::validate_bitboards_consistency(test_pos) ? "✓" : "✗") << std::endl;
    std::cout << "  Piece counts:   " << (Debug::validate_piece_counts_consistency(test_pos) ? "✓" : "✗") << std::endl;
    std::cout << "  Piece lists:    " << (Debug::validate_piece_lists_consistency(test_pos) ? "✓" : "✗") << std::endl;
    std::cout << "  Material scores:" << (Debug::validate_material_scores_consistency(test_pos) ? "✓" : "✗") << std::endl;
    std::cout << "  King squares:   " << (Debug::validate_king_squares_consistency(test_pos) ? "✓" : "✗") << std::endl;
    std::cout << "  Zobrist hash:   " << (Debug::validate_zobrist_consistency(test_pos) ? "✓" : "✗") << std::endl;
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    std::cout << "\nUsage:" << std::endl;
    std::cout << "- Use Debug::validate_position_consistency(pos, expected_fen) for comprehensive validation" << std::endl;
    std::cout << "- Use individual Debug::validate_*_consistency(pos) functions for specific checks" << std::endl;
    std::cout << "- All functions return true if consistent, false if inconsistent" << std::endl;
    std::cout << "- Assertions will trigger with detailed error messages when inconsistencies are found" << std::endl;
    
    return 0;
}
