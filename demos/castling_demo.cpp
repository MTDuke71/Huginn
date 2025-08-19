#include "debug.hpp"
#include "position.hpp"
#include "init.hpp"
#include <iostream>

int main() {
    // Initialize the engine
    Huginn::init();
    
    std::cout << "=== Castling Validation Demo ===\n" << std::endl;
    
    Position pos;
    
    // Test 1: Valid castling scenario
    std::cout << "1. Testing valid castling scenario:" << std::endl;
    pos.set_startpos();
    if (Debug::validate_castling_consistency(pos)) {
        std::cout << "✅ PASSED: Starting position has valid castling rights\n" << std::endl;
    } else {
        std::cout << "❌ FAILED: Starting position should have valid castling rights\n" << std::endl;
    }
    
    // Test 2: Invalid castling - King moved but rights still set
    std::cout << "2. Testing invalid castling (King moved but rights still claim castling possible):" << std::endl;
    pos.reset();
    pos.castling_rights = CASTLE_WK;  // Claim White can castle kingside
    pos.board[sq(File::F, Rank::R1)] = Piece::WhiteKing;    // King on f1 (moved)
    pos.board[sq(File::H, Rank::R1)] = Piece::WhiteRook;    // Rook on h1 (correct)
    
    if (!Debug::validate_castling_consistency(pos)) {
        std::cout << "✅ PASSED: Correctly detected invalid castling (King not on e1)\n" << std::endl;
    } else {
        std::cout << "❌ FAILED: Should have detected King not on starting square\n" << std::endl;
    }
    
    // Test 3: Invalid castling - Rook moved but rights still set
    std::cout << "3. Testing invalid castling (Rook moved but rights still claim castling possible):" << std::endl;
    pos.reset();
    pos.castling_rights = CASTLE_BQ;  // Claim Black can castle queenside
    pos.board[sq(File::E, Rank::R8)] = Piece::BlackKing;    // King on e8 (correct)
    pos.board[sq(File::B, Rank::R8)] = Piece::BlackRook;    // Rook on b8 (moved from a8)
    
    if (!Debug::validate_castling_consistency(pos)) {
        std::cout << "✅ PASSED: Correctly detected invalid castling (Rook not on a8)\n" << std::endl;
    } else {
        std::cout << "❌ FAILED: Should have detected Rook not on starting square\n" << std::endl;
    }
    
    // Test 4: Partial castling rights (valid scenario)
    std::cout << "4. Testing partial castling rights (only some castling allowed):" << std::endl;
    pos.reset();
    pos.castling_rights = CASTLE_WK | CASTLE_BK;  // Only kingside castling for both colors
    pos.board[sq(File::E, Rank::R1)] = Piece::WhiteKing;
    pos.board[sq(File::H, Rank::R1)] = Piece::WhiteRook;
    pos.board[sq(File::E, Rank::R8)] = Piece::BlackKing;
    pos.board[sq(File::H, Rank::R8)] = Piece::BlackRook;
    // No queenside rooks - should be fine since queenside castling rights not set
    
    if (Debug::validate_castling_consistency(pos)) {
        std::cout << "✅ PASSED: Partial castling rights correctly validated\n" << std::endl;
    } else {
        std::cout << "❌ FAILED: Partial castling rights should be valid\n" << std::endl;
    }
    
    // Test 5: Test with a real FEN position that should be valid
    std::cout << "5. Testing Kiwipete position (famous test position with all castling rights):" << std::endl;
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    pos.set_from_fen(kiwipete_fen);
    
    if (Debug::validate_castling_consistency(pos)) {
        std::cout << "✅ PASSED: Kiwipete position has valid castling setup" << std::endl;
    } else {
        std::cout << "❌ FAILED: Kiwipete position should have valid castling setup" << std::endl;
    }
    
    std::cout << "\n=== Castling Validation Demo Complete ===\n" << std::endl;
    std::cout << "The debug system can now detect when castling rights are inconsistent" << std::endl;
    std::cout << "with actual piece positions, helping catch position corruption bugs!" << std::endl;
    
    return 0;
}
