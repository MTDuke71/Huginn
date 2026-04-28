#include <gtest/gtest.h>
#include "position.hpp"
#include "attack_detection.hpp"
#include "board120.hpp"
#include "chess_types.hpp"

class SqAttackedTest : public ::testing::Test {
protected:
    void SetUp() override {
        pos.reset();
    }

    // Manual test setup helper: place a piece and keep bitboard state in sync.
    // Position::set() is a low-level mailbox-only setter; bitboards are rebuilt
    // from board[] by rebuild_counts(). Tests that hand-craft positions need
    // this so the bitboard-based SqAttacked sees the pieces.
    void place(int square, Piece p) {
        pos.set(square, p);
        pos.rebuild_counts();
    }

    Position pos;
};

TEST_F(SqAttackedTest, PawnAttacks) {
    // Test white pawn attacks
    place(sq(File::E, Rank::R4), Piece::WhitePawn);

    // White pawn on e4 should attack d5 and f5
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::D, Rank::R5), pos, Color::White));
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::F, Rank::R5), pos, Color::White));

    // Should not attack other squares
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::E, Rank::R5), pos, Color::White)); // straight ahead
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::D, Rank::R4), pos, Color::White)); // same rank
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::D, Rank::R3), pos, Color::White)); // wrong direction

    // Test black pawn attacks
    pos.reset();
    place(sq(File::D, Rank::R5), Piece::BlackPawn);

    // Black pawn on d5 should attack c4 and e4
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::C, Rank::R4), pos, Color::Black));
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::E, Rank::R4), pos, Color::Black));

    // Should not attack other squares
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::D, Rank::R4), pos, Color::Black)); // straight ahead
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::C, Rank::R5), pos, Color::Black)); // same rank
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::C, Rank::R6), pos, Color::Black)); // wrong direction
}

TEST_F(SqAttackedTest, KnightAttacks) {
    place(sq(File::E, Rank::R4), Piece::WhiteKnight);

    // Knight should attack all 8 knight squares from e4
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::D, Rank::R6), pos, Color::White)); // d6
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::F, Rank::R6), pos, Color::White)); // f6
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::C, Rank::R5), pos, Color::White)); // c5
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::G, Rank::R5), pos, Color::White)); // g5
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::C, Rank::R3), pos, Color::White)); // c3
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::G, Rank::R3), pos, Color::White)); // g3
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::D, Rank::R2), pos, Color::White)); // d2
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::F, Rank::R2), pos, Color::White)); // f2

    // Should not attack adjacent squares
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::E, Rank::R5), pos, Color::White)); // e5
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::D, Rank::R4), pos, Color::White)); // d4
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::E, Rank::R3), pos, Color::White)); // e3
}

TEST_F(SqAttackedTest, KingAttacks) {
    place(sq(File::E, Rank::R4), Piece::WhiteKing);

    // King should attack all 8 adjacent squares
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::D, Rank::R5), pos, Color::White)); // d5
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::E, Rank::R5), pos, Color::White)); // e5
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::F, Rank::R5), pos, Color::White)); // f5
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::D, Rank::R4), pos, Color::White)); // d4
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::F, Rank::R4), pos, Color::White)); // f4
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::D, Rank::R3), pos, Color::White)); // d3
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::E, Rank::R3), pos, Color::White)); // e3
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::F, Rank::R3), pos, Color::White)); // f3

    // Should not attack distant squares
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::E, Rank::R6), pos, Color::White)); // e6
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::C, Rank::R4), pos, Color::White)); // c4
}

TEST_F(SqAttackedTest, RookAttacks) {
    place(sq(File::E, Rank::R4), Piece::WhiteRook);

    // Rook should attack entire rank and file
    for (int r = 1; r <= 8; ++r) {
        if (r != 4) {
            EXPECT_TRUE(Huginn::SqAttacked(sq(File::E, static_cast<Rank>(r-1)), pos, Color::White));
        }
    }
    for (int f = 0; f < 8; ++f) {
        if (f != 4) {
            EXPECT_TRUE(Huginn::SqAttacked(sq(static_cast<File>(f), Rank::R4), pos, Color::White));
        }
    }

    // Should not attack diagonal squares
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::D, Rank::R5), pos, Color::White)); // d5
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::F, Rank::R3), pos, Color::White)); // f3
}

TEST_F(SqAttackedTest, BishopAttacks) {
    place(sq(File::E, Rank::R4), Piece::WhiteBishop);

    // NE diagonal
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::F, Rank::R5), pos, Color::White)); // f5
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::G, Rank::R6), pos, Color::White)); // g6
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::H, Rank::R7), pos, Color::White)); // h7

    // NW diagonal
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::D, Rank::R5), pos, Color::White)); // d5
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::C, Rank::R6), pos, Color::White)); // c6
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::B, Rank::R7), pos, Color::White)); // b7
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::A, Rank::R8), pos, Color::White)); // a8

    // SE diagonal
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::F, Rank::R3), pos, Color::White)); // f3
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::G, Rank::R2), pos, Color::White)); // g2
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::H, Rank::R1), pos, Color::White)); // h1

    // SW diagonal
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::D, Rank::R3), pos, Color::White)); // d3
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::C, Rank::R2), pos, Color::White)); // c2
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::B, Rank::R1), pos, Color::White)); // b1

    // Should not attack rank/file squares
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::E, Rank::R5), pos, Color::White)); // e5
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::D, Rank::R4), pos, Color::White)); // d4
}

TEST_F(SqAttackedTest, QueenAttacks) {
    place(sq(File::E, Rank::R4), Piece::WhiteQueen);

    // Queen should attack like both rook and bishop
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::E, Rank::R1), pos, Color::White)); // e1
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::E, Rank::R8), pos, Color::White)); // e8
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::A, Rank::R4), pos, Color::White)); // a4
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::H, Rank::R4), pos, Color::White)); // h4
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::A, Rank::R8), pos, Color::White)); // a8
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::H, Rank::R7), pos, Color::White)); // h7
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::H, Rank::R1), pos, Color::White)); // h1
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::B, Rank::R1), pos, Color::White)); // b1
}

TEST_F(SqAttackedTest, BlockedAttacks) {
    // Test that pieces block sliding piece attacks
    pos.set(sq(File::E, Rank::R4), Piece::WhiteRook);
    pos.set(sq(File::E, Rank::R6), Piece::BlackPawn); // Blocking piece
    pos.rebuild_counts();

    // Rook should attack e5 (before blocking piece)
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::E, Rank::R5), pos, Color::White));

    // But not e7 or e8 (blocked by pawn on e6)
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::E, Rank::R7), pos, Color::White));
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::E, Rank::R8), pos, Color::White));

    // Test diagonal blocking
    pos.reset();
    pos.set(sq(File::E, Rank::R4), Piece::WhiteBishop);
    pos.set(sq(File::F, Rank::R5), Piece::BlackPawn); // Blocking piece
    pos.rebuild_counts();

    // Bishop should not attack g6 or h7 (blocked by pawn on f5)
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::G, Rank::R6), pos, Color::White));
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::H, Rank::R7), pos, Color::White));
}

TEST_F(SqAttackedTest, OwnPiecesDoNotAttack) {
    place(sq(File::E, Rank::R4), Piece::WhiteRook);

    // Should not be attacked by black pieces (there are none)
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::E, Rank::R5), pos, Color::Black));
    EXPECT_FALSE(Huginn::SqAttacked(sq(File::D, Rank::R4), pos, Color::Black));

    // But should be attacked by white pieces
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::E, Rank::R5), pos, Color::White));
    EXPECT_TRUE(Huginn::SqAttacked(sq(File::D, Rank::R4), pos, Color::White));
}

TEST_F(SqAttackedTest, OffboardSquares) {
    place(sq(File::E, Rank::R4), Piece::WhiteQueen);

    // These should return false for offboard squares
    EXPECT_FALSE(Huginn::SqAttacked(-1, pos, Color::White));
    EXPECT_FALSE(Huginn::SqAttacked(0, pos, Color::White));   // Offboard frame
    EXPECT_FALSE(Huginn::SqAttacked(120, pos, Color::White)); // Beyond board
}
