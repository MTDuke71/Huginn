#include <gtest/gtest.h>
#include "chess_types.hpp"  // Piece, Color, PieceType, helpers

TEST(ChessTypes, MakeAndDecompose) {
    Piece p = make_piece(Color::White, PieceType::Knight);
    EXPECT_EQ(color_of(p), Color::White);
    EXPECT_EQ(type_of(p),  PieceType::Knight);

    EXPECT_TRUE(is_none(make_piece(Color::None, PieceType::Queen)));
    EXPECT_TRUE(is_none(make_piece(Color::White, PieceType::None)));
}

TEST(ChessTypes, CharConversions) {
    EXPECT_EQ(to_char(Piece::WhitePawn),   'P');
    EXPECT_EQ(to_char(Piece::BlackKnight), 'n');
    EXPECT_EQ(to_char(Piece::WhiteKing),   'K');

    EXPECT_EQ(from_char('q'), Piece::BlackQueen);
    EXPECT_EQ(from_char('R'), Piece::WhiteRook);
    EXPECT_EQ(from_char('.'), Piece::None);
}

TEST(ChessTypes, ValuesAndClassifiers) {
    EXPECT_EQ(value_of(Piece::WhitePawn),    100);
    EXPECT_EQ(value_of(Piece::BlackKnight),  320);
    EXPECT_EQ(value_of(Piece::WhiteBishop),  330);
    EXPECT_EQ(value_of(Piece::WhiteRook),    500);
    EXPECT_EQ(value_of(Piece::BlackQueen),   900);
    EXPECT_GT(value_of(Piece::WhiteKing),  10000);

    EXPECT_TRUE(is_minor(PieceType::Knight));
    EXPECT_TRUE(is_minor(PieceType::Bishop));
    EXPECT_TRUE(is_major(PieceType::Rook));
    EXPECT_TRUE(is_major(PieceType::Queen));
    EXPECT_TRUE(is_slider(PieceType::Bishop));
    EXPECT_TRUE(is_slider(PieceType::Rook));
    EXPECT_TRUE(is_slider(PieceType::Queen));
    EXPECT_FALSE(is_slider(PieceType::Pawn));
    EXPECT_FALSE(is_slider(PieceType::King));
}
