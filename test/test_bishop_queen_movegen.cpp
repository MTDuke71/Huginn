#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen_enhanced.hpp"

TEST(BishopMoveGen, SingleBishopCenter) {
    Position pos; pos.reset();
    // Place a white bishop on d4
    pos.set(sq(File::D, Rank::R4), Piece::WhiteBishop);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    // Assert bishop piece list and count are correct
    ASSERT_EQ(pos.pCount[int(Color::White)][int(PieceType::Bishop)], 1);
    ASSERT_EQ(pos.pList[int(Color::White)][int(PieceType::Bishop)][0], sq(File::D, Rank::R4));
    
    MoveList moves; generate_pseudo_legal_moves(pos, moves);
    int bishop_moves = 0;
    for (const auto& m : moves.v) {
        if (pos.at(m.get_from()) == Piece::WhiteBishop)
            ++bishop_moves;
    }
    // Should be 13 (bishop on d4 can move to 13 diagonal squares)
    EXPECT_EQ(bishop_moves, 13);
}

TEST(BishopMoveGen, BishopBlockedByOwnPiece) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteBishop);
    pos.set(sq(File::F, Rank::R6), Piece::WhitePawn); // Block NE diagonal
    pos.set(sq(File::B, Rank::R6), Piece::WhitePawn); // Block NW diagonal
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    MoveList moves; generate_pseudo_legal_moves(pos, moves);
    int bishop_moves = 0;
    for (const auto& m : moves.v) {
        if (pos.at(m.get_from()) == Piece::WhiteBishop)
            ++bishop_moves;
    }
    // Should be less than 13 due to blocking
    EXPECT_LT(bishop_moves, 13);
}

TEST(BishopMoveGen, BishopCapturesOpponent) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteBishop);
    pos.set(sq(File::F, Rank::R6), Piece::BlackPawn); // Capture NE
    pos.set(sq(File::B, Rank::R2), Piece::BlackKnight); // Capture SW
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    MoveList moves; generate_pseudo_legal_moves(pos, moves);
    int capture_moves = 0;
    for (const auto& m : moves.v) {
        if (pos.at(m.get_from()) == Piece::WhiteBishop && m.get_to() == sq(File::F, Rank::R6))
            ++capture_moves;
        if (pos.at(m.get_from()) == Piece::WhiteBishop && m.get_to() == sq(File::B, Rank::R2))
            ++capture_moves;
    }
    EXPECT_EQ(capture_moves, 2);
}

TEST(QueenMoveGen, SingleQueenCenter) {
    Position pos; pos.reset();
    // Place a white queen on d4
    pos.set(sq(File::D, Rank::R4), Piece::WhiteQueen);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    // Assert queen piece list and count are correct
    ASSERT_EQ(pos.pCount[int(Color::White)][int(PieceType::Queen)], 1);
    ASSERT_EQ(pos.pList[int(Color::White)][int(PieceType::Queen)][0], sq(File::D, Rank::R4));
    
    MoveList moves; generate_pseudo_legal_moves(pos, moves);
    int queen_moves = 0;
    for (const auto& m : moves.v) {
        if (pos.at(m.get_from()) == Piece::WhiteQueen)
            ++queen_moves;
    }
    // Should be 27 (14 rook moves + 13 bishop moves)
    EXPECT_EQ(queen_moves, 27);
}

TEST(QueenMoveGen, QueenBlockedByOwnPiece) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteQueen);
    pos.set(sq(File::D, Rank::R6), Piece::WhitePawn); // Block north
    pos.set(sq(File::F, Rank::R4), Piece::WhitePawn); // Block east
    pos.set(sq(File::F, Rank::R6), Piece::WhitePawn); // Block NE diagonal
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    MoveList moves; generate_pseudo_legal_moves(pos, moves);
    int queen_moves = 0;
    for (const auto& m : moves.v) {
        if (pos.at(m.get_from()) == Piece::WhiteQueen)
            ++queen_moves;
    }
    // Should be less than 27 due to blocking
    EXPECT_LT(queen_moves, 27);
}

TEST(QueenMoveGen, QueenCapturesOpponent) {
    Position pos; pos.reset();
    pos.set(sq(File::D, Rank::R4), Piece::WhiteQueen);
    pos.set(sq(File::D, Rank::R6), Piece::BlackPawn); // Capture north
    pos.set(sq(File::F, Rank::R4), Piece::BlackKnight); // Capture east
    pos.set(sq(File::F, Rank::R6), Piece::BlackRook); // Capture NE diagonal
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    MoveList moves; generate_pseudo_legal_moves(pos, moves);
    int capture_moves = 0;
    for (const auto& m : moves.v) {
        if (pos.at(m.get_from()) == Piece::WhiteQueen && m.get_to() == sq(File::D, Rank::R6))
            ++capture_moves;
        if (pos.at(m.get_from()) == Piece::WhiteQueen && m.get_to() == sq(File::F, Rank::R4))
            ++capture_moves;
        if (pos.at(m.get_from()) == Piece::WhiteQueen && m.get_to() == sq(File::F, Rank::R6))
            ++capture_moves;
    }
    EXPECT_EQ(capture_moves, 3);
}

TEST(SlidingPieceMoveGen, AllSlidingPiecesTogether) {
    Position pos; pos.reset();
    // Place all sliding pieces on central squares
    pos.set(sq(File::D, Rank::R4), Piece::WhiteRook);
    pos.set(sq(File::E, Rank::R4), Piece::WhiteBishop);
    pos.set(sq(File::F, Rank::R4), Piece::WhiteQueen);
    pos.side_to_move = Color::White;
    pos.rebuild_counts();
    
    MoveList moves; generate_pseudo_legal_moves(pos, moves);
    int rook_moves = 0, bishop_moves = 0, queen_moves = 0;
    for (const auto& m : moves.v) {
        if (pos.at(m.get_from()) == Piece::WhiteRook) ++rook_moves;
        if (pos.at(m.get_from()) == Piece::WhiteBishop) ++bishop_moves;
        if (pos.at(m.get_from()) == Piece::WhiteQueen) ++queen_moves;
    }
    
    // Verify each piece type generates expected moves
    EXPECT_GT(rook_moves, 0);   // Rook should have moves
    EXPECT_GT(bishop_moves, 0); // Bishop should have moves
    EXPECT_GT(queen_moves, 0);  // Queen should have moves
    
    // Queen should have more moves than rook or bishop individually
    EXPECT_GT(queen_moves, rook_moves);
    EXPECT_GT(queen_moves, bishop_moves);
}
