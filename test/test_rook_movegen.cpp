#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen_enhanced.hpp"

TEST(RookMoveGen, SingleRookCenter) {
    Position pos; pos.reset();
    // Place a white rook on d4
    pos.set(sq(File::D, Rank::R4), Piece::WhiteRook);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    // Assert rook piece list and count are correct
    ASSERT_EQ(pos.pCount[int(Color::White)][int(PieceType::Rook)], 1);
    ASSERT_EQ(pos.pList[int(Color::White)][int(PieceType::Rook)][0], sq(File::D, Rank::R4));
    
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    int rook_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteRook)
            ++rook_moves;
    }
    // Should be 14 (7 in each direction, minus the square itself)
    EXPECT_EQ(rook_moves, 14);
}

TEST(RookMoveGen, RookBlockedByOwnPiece) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteRook);
    pos.set(sq(File::D, Rank::R6), Piece::WhitePawn); // Block north
    pos.set(sq(File::F, Rank::R4), Piece::WhitePawn); // Block east
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    int rook_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteRook)
            ++rook_moves;
    }
    // Should be less than 14 due to blocking
    EXPECT_LT(rook_moves, 14);
}

TEST(RookMoveGen, RookCapturesOpponent) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteRook);
    pos.set(sq(File::D, Rank::R6), Piece::BlackPawn); // Capture north
    pos.set(sq(File::F, Rank::R4), Piece::BlackKnight); // Capture east
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    S_MOVELIST moves; 
    generate_all_moves(pos, moves);
    int capture_moves = 0;
    for (int i = 0; i < moves.count; i++) {
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteRook && moves.moves[i].get_to() == sq(File::D, Rank::R6))
            ++capture_moves;
        if (pos.at(moves.moves[i].get_from()) == Piece::WhiteRook && moves.moves[i].get_to() == sq(File::F, Rank::R4))
            ++capture_moves;
    }
    EXPECT_EQ(capture_moves, 2);
}
