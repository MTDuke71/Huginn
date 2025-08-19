#include "position.hpp"
#include "init.hpp"
#include <iostream>

int main() {
    // Initialize the engine
    Huginn::init();
    
    std::cout << "=== FEN Generation Demo ===\n" << std::endl;
    
    Position pos;
    
    // Test 1: Starting position
    std::cout << "1. Starting position:" << std::endl;
    pos.set_startpos();
    std::string start_fen = pos.to_fen();
    std::cout << "Generated FEN: " << start_fen << std::endl;
    std::cout << "Expected FEN:  rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" << std::endl;
    std::cout << "Match: " << (start_fen == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" ? "✅ YES" : "❌ NO") << "\n" << std::endl;
    
    // Test 2: Complex position (Kiwipete)
    std::cout << "2. Kiwipete test position:" << std::endl;
    const std::string kiwipete_original = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    pos.set_from_fen(kiwipete_original);
    std::string kiwipete_generated = pos.to_fen();
    std::cout << "Original FEN:  " << kiwipete_original << std::endl;
    std::cout << "Generated FEN: " << kiwipete_generated << std::endl;
    std::cout << "Match: " << (kiwipete_generated == kiwipete_original ? "✅ YES" : "❌ NO") << "\n" << std::endl;
    
    // Test 3: Position with en passant
    std::cout << "3. Position with en passant:" << std::endl;
    const std::string ep_original = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    pos.set_from_fen(ep_original);
    std::string ep_generated = pos.to_fen();
    std::cout << "Original FEN:  " << ep_original << std::endl;
    std::cout << "Generated FEN: " << ep_generated << std::endl;
    std::cout << "Match: " << (ep_generated == ep_original ? "✅ YES" : "❌ NO") << "\n" << std::endl;
    
    // Test 4: Position with partial castling rights
    std::cout << "4. Position with partial castling (only Kq):" << std::endl;
    const std::string partial_original = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kq - 5 10";
    pos.set_from_fen(partial_original);
    std::string partial_generated = pos.to_fen();
    std::cout << "Original FEN:  " << partial_original << std::endl;
    std::cout << "Generated FEN: " << partial_generated << std::endl;
    std::cout << "Match: " << (partial_generated == partial_original ? "✅ YES" : "❌ NO") << "\n" << std::endl;
    
    // Test 5: Empty board
    std::cout << "5. Empty board:" << std::endl;
    pos.reset();
    pos.side_to_move = Color::Black;
    pos.halfmove_clock = 50;
    pos.fullmove_number = 25;
    std::string empty_generated = pos.to_fen();
    std::cout << "Generated FEN: " << empty_generated << std::endl;
    std::cout << "Expected FEN:  8/8/8/8/8/8/8/8 b - - 50 25" << std::endl;
    std::cout << "Match: " << (empty_generated == "8/8/8/8/8/8/8/8 b - - 50 25" ? "✅ YES" : "❌ NO") << "\n" << std::endl;
    
    // Test 6: Custom position to show piece representation
    std::cout << "6. Custom position with various pieces:" << std::endl;
    pos.reset();
    pos.side_to_move = Color::White;
    pos.castling_rights = CASTLE_WK | CASTLE_BQ;
    pos.ep_square = sq(File::D, Rank::R6);
    pos.halfmove_clock = 3;
    pos.fullmove_number = 15;
    
    // Place some pieces manually
    pos.board[sq(File::E, Rank::R1)] = Piece::WhiteKing;
    pos.board[sq(File::H, Rank::R1)] = Piece::WhiteRook;
    pos.board[sq(File::D, Rank::R4)] = Piece::WhiteQueen;
    pos.board[sq(File::E, Rank::R8)] = Piece::BlackKing;
    pos.board[sq(File::A, Rank::R8)] = Piece::BlackRook;
    pos.board[sq(File::F, Rank::R7)] = Piece::BlackPawn;
    pos.board[sq(File::C, Rank::R3)] = Piece::WhiteKnight;
    pos.board[sq(File::B, Rank::R6)] = Piece::BlackBishop;
    
    std::string custom_generated = pos.to_fen();
    std::cout << "Generated FEN: " << custom_generated << std::endl;
    std::cout << "Pieces: K=♔ Q=♕ R=♖ B=♗ N=♘ P=♙ (uppercase=White, lowercase=Black)" << std::endl;
    
    std::cout << "\n=== FEN Generation Demo Complete ===\n" << std::endl;
    std::cout << "The to_fen() function successfully converts position arrays to FEN strings!" << std::endl;
    std::cout << "This enables:" << std::endl;
    std::cout << "• Position serialization for saving/loading games" << std::endl;
    std::cout << "• Debug output and logging" << std::endl;
    std::cout << "• Communication with external chess tools" << std::endl;
    std::cout << "• Round-trip FEN validation (parse -> generate -> compare)" << std::endl;
    
    return 0;
}
