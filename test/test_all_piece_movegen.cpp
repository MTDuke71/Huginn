#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen_enhanced.hpp"

// Test that all implemented piece types work together
TEST(AllPieceMoveGen, AllImplementedPiecesGenerateMoves) {
    Position pos;
    pos.reset();
    
    // Place various pieces on the board
    pos.set(sq(File::D, Rank::R4), Piece::WhiteKing);
    pos.set(sq(File::E, Rank::R4), Piece::WhiteQueen);
    pos.set(sq(File::F, Rank::R4), Piece::WhiteRook);
    pos.set(sq(File::G, Rank::R4), Piece::WhiteBishop);
    pos.set(sq(File::H, Rank::R4), Piece::WhiteKnight);
    
    // Place some enemy pieces for captures
    pos.set(sq(File::D, Rank::R6), Piece::BlackPawn);
    pos.set(sq(File::E, Rank::R6), Piece::BlackRook);
    
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // Count moves by piece type
    int king_moves = 0, queen_moves = 0, rook_moves = 0, bishop_moves = 0, knight_moves = 0;
    
    for (int i = 0; i < moves.count; i++) {
        Piece piece = pos.at(moves.moves[i].get_from());
        PieceType type = type_of(piece);
        
        switch (type) {
            case PieceType::King:   king_moves++; break;
            case PieceType::Queen:  queen_moves++; break;
            case PieceType::Rook:   rook_moves++; break;
            case PieceType::Bishop: bishop_moves++; break;
            case PieceType::Knight: knight_moves++; break;
            default: break;
        }
    }
    
    // Verify each piece type can move
    EXPECT_GT(king_moves, 0) << "King should be able to move";
    EXPECT_GT(queen_moves, 0) << "Queen should be able to move";  
    EXPECT_GT(rook_moves, 0) << "Rook should be able to move";
    EXPECT_GT(bishop_moves, 0) << "Bishop should be able to move";
    EXPECT_GT(knight_moves, 0) << "Knight should be able to move";
    
    // Check total moves are reasonable
    EXPECT_GT(moves.size(), 20) << "Should have many possible moves";
    
    std::cout << "Move counts - King: " << king_moves 
              << ", Queen: " << queen_moves 
              << ", Rook: " << rook_moves
              << ", Bishop: " << bishop_moves 
              << ", Knight: " << knight_moves 
              << ", Total: " << moves.size() << std::endl;
}

// Test king-specific move generation in isolation
TEST(AllPieceMoveGen, KingMoveCountCorrect) {
    Position pos;
    pos.reset();
    
    // Place only a king on d4
    pos.set(sq(File::D, Rank::R4), Piece::WhiteKing);
    pos.rebuild_counts();
    pos.side_to_move = Color::White;
    
    S_MOVELIST moves;
    generate_all_moves(pos, moves);
    
    // King on d4 should have exactly 8 moves
    EXPECT_EQ(moves.size(), 8);
}
