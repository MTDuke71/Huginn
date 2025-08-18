#include <gtest/gtest.h>
#include "../src/position.hpp"
#include "../src/chess_types.hpp"

class FENTest : public ::testing::Test {
protected:
    Position pos;
};

TEST_F(FENTest, ParseStartingPosition) {
    const std::string start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    
    ASSERT_TRUE(pos.set_from_fen(start_fen));
    
    // Check piece placement
    EXPECT_EQ(pos.at(sq(File::A, Rank::R1)), Piece::WhiteRook);
    EXPECT_EQ(pos.at(sq(File::B, Rank::R1)), Piece::WhiteKnight);
    EXPECT_EQ(pos.at(sq(File::C, Rank::R1)), Piece::WhiteBishop);
    EXPECT_EQ(pos.at(sq(File::D, Rank::R1)), Piece::WhiteQueen);
    EXPECT_EQ(pos.at(sq(File::E, Rank::R1)), Piece::WhiteKing);
    EXPECT_EQ(pos.at(sq(File::F, Rank::R1)), Piece::WhiteBishop);
    EXPECT_EQ(pos.at(sq(File::G, Rank::R1)), Piece::WhiteKnight);
    EXPECT_EQ(pos.at(sq(File::H, Rank::R1)), Piece::WhiteRook);
    
    // Check white pawns
    for (int f = 0; f < 8; ++f) {
        EXPECT_EQ(pos.at(sq(static_cast<File>(f), Rank::R2)), Piece::WhitePawn);
    }
    
    // Check empty squares
    for (int f = 0; f < 8; ++f) {
        for (int r = 2; r < 6; ++r) {
            EXPECT_EQ(pos.at(sq(static_cast<File>(f), static_cast<Rank>(r))), Piece::None);
        }
    }
    
    // Check black pawns
    for (int f = 0; f < 8; ++f) {
        EXPECT_EQ(pos.at(sq(static_cast<File>(f), Rank::R7)), Piece::BlackPawn);
    }
    
    // Check black pieces
    EXPECT_EQ(pos.at(sq(File::A, Rank::R8)), Piece::BlackRook);
    EXPECT_EQ(pos.at(sq(File::B, Rank::R8)), Piece::BlackKnight);
    EXPECT_EQ(pos.at(sq(File::C, Rank::R8)), Piece::BlackBishop);
    EXPECT_EQ(pos.at(sq(File::D, Rank::R8)), Piece::BlackQueen);
    EXPECT_EQ(pos.at(sq(File::E, Rank::R8)), Piece::BlackKing);
    EXPECT_EQ(pos.at(sq(File::F, Rank::R8)), Piece::BlackBishop);
    EXPECT_EQ(pos.at(sq(File::G, Rank::R8)), Piece::BlackKnight);
    EXPECT_EQ(pos.at(sq(File::H, Rank::R8)), Piece::BlackRook);
    
    // Check game state
    EXPECT_EQ(pos.side_to_move, Color::White);
    EXPECT_EQ(pos.castling_rights, CASTLE_ALL);
    EXPECT_EQ(pos.ep_square, -1);
    EXPECT_EQ(pos.halfmove_clock, 0);
    EXPECT_EQ(pos.fullmove_number, 1);
}

TEST_F(FENTest, ParseKiwiPetePosition) {
    // Kiwipete test position - good for testing castling, en passant, etc.
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    // Check some key pieces
    EXPECT_EQ(pos.at(sq(File::A, Rank::R8)), Piece::BlackRook);
    EXPECT_EQ(pos.at(sq(File::E, Rank::R8)), Piece::BlackKing);
    EXPECT_EQ(pos.at(sq(File::H, Rank::R8)), Piece::BlackRook);
    EXPECT_EQ(pos.at(sq(File::A, Rank::R1)), Piece::WhiteRook);
    EXPECT_EQ(pos.at(sq(File::E, Rank::R1)), Piece::WhiteKing);
    EXPECT_EQ(pos.at(sq(File::H, Rank::R1)), Piece::WhiteRook);
    
    // Check game state
    EXPECT_EQ(pos.side_to_move, Color::White);
    EXPECT_EQ(pos.castling_rights, CASTLE_ALL);
    EXPECT_EQ(pos.ep_square, -1);
    EXPECT_EQ(pos.halfmove_clock, 0);
    EXPECT_EQ(pos.fullmove_number, 1);
}

TEST_F(FENTest, ParsePositionWithEnPassant) {
    // Position with en passant square
    const std::string fen_with_ep = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    
    ASSERT_TRUE(pos.set_from_fen(fen_with_ep));
    
    EXPECT_EQ(pos.side_to_move, Color::Black);
    EXPECT_EQ(pos.castling_rights, CASTLE_ALL);
    EXPECT_EQ(pos.ep_square, sq(File::E, Rank::R3));
    EXPECT_EQ(pos.halfmove_clock, 0);
    EXPECT_EQ(pos.fullmove_number, 1);
}

TEST_F(FENTest, ParsePositionWithPartialCastling) {
    // Position with only some castling rights
    const std::string fen_partial_castle = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kq - 0 1";
    
    ASSERT_TRUE(pos.set_from_fen(fen_partial_castle));
    
    EXPECT_EQ(pos.castling_rights, CASTLE_WK | CASTLE_BQ);
}

TEST_F(FENTest, ParsePositionWithNoCastling) {
    // Position with no castling rights
    const std::string fen_no_castle = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1";
    
    ASSERT_TRUE(pos.set_from_fen(fen_no_castle));
    
    EXPECT_EQ(pos.castling_rights, CASTLE_NONE);
}

TEST_F(FENTest, ParsePositionWithMoveCounters) {
    // Position with different move counters
    const std::string fen_counters = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 5 10";
    
    ASSERT_TRUE(pos.set_from_fen(fen_counters));
    
    EXPECT_EQ(pos.halfmove_clock, 5);
    EXPECT_EQ(pos.fullmove_number, 10);
}

TEST_F(FENTest, SetStartPositionUsesFEN) {
    pos.set_startpos();
    
    // Verify it matches the standard starting position
    EXPECT_EQ(pos.at(sq(File::E, Rank::R1)), Piece::WhiteKing);
    EXPECT_EQ(pos.at(sq(File::E, Rank::R8)), Piece::BlackKing);
    EXPECT_EQ(pos.side_to_move, Color::White);
    EXPECT_EQ(pos.castling_rights, CASTLE_ALL);
    EXPECT_EQ(pos.ep_square, -1);
    EXPECT_EQ(pos.halfmove_clock, 0);
    EXPECT_EQ(pos.fullmove_number, 1);
    
    // Check that piece counts are correctly updated
    EXPECT_EQ(pos.piece_counts[int(PieceType::Pawn)], 16);   // 8 white + 8 black pawns
    EXPECT_EQ(pos.piece_counts[int(PieceType::Rook)], 4);    // 2 white + 2 black rooks
    EXPECT_EQ(pos.piece_counts[int(PieceType::Knight)], 4);  // 2 white + 2 black knights
    EXPECT_EQ(pos.piece_counts[int(PieceType::Bishop)], 4);  // 2 white + 2 black bishops
    EXPECT_EQ(pos.piece_counts[int(PieceType::Queen)], 2);   // 1 white + 1 black queen
    EXPECT_EQ(pos.piece_counts[int(PieceType::King)], 2);    // 1 white + 1 black king
}

TEST_F(FENTest, InvalidFENFormats) {
    // Test various invalid FEN strings
    EXPECT_FALSE(pos.set_from_fen(""));  // Empty string
    EXPECT_FALSE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"));  // Missing fields
    EXPECT_FALSE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq"));  // Too few fields
    EXPECT_FALSE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 extra"));  // Too many fields
    EXPECT_FALSE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 1"));  // Invalid side
    EXPECT_FALSE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkX - 0 1"));  // Invalid castling
    EXPECT_FALSE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq z9 0 1"));  // Invalid ep square
    EXPECT_FALSE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - abc 1"));  // Invalid halfmove
    EXPECT_FALSE(pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 xyz"));  // Invalid fullmove
}
