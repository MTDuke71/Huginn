#include <gtest/gtest.h>
#include "position.hpp"
#include "chess_types.hpp"
#include "bitboard.hpp"

class IncrementalUpdateTest : public ::testing::Test {
protected:
    Position pos;

    void SetUp() override {
        pos.set_startpos();
    }

    static int total_count(const Position& p, PieceType pt) {
        return popcount(p.piece_bitboards[int(Color::White)][int(pt)])
             + popcount(p.piece_bitboards[int(Color::Black)][int(pt)]);
    }
};

TEST_F(IncrementalUpdateTest, MakeUnmakeMaintainsState) {
    auto initial_king_sq = pos.king_sq;
    auto initial_white_pawns = pos.get_white_pawns();
    int initial_pawn_count = total_count(pos, PieceType::Pawn);

    // Make a simple pawn move: e2-e4
    S_MOVE move = make_move(sq64(File::E, Rank::R2), sq64(File::E, Rank::R4));
    int move_result = pos.MakeMove(move);
    EXPECT_EQ(move_result, 1) << "Move should be legal";

    // White pawn should have moved from e2 to e4
    int e2_sq64 = MAILBOX_MAPS.to64[sq(File::E, Rank::R2)];
    int e4_sq64 = MAILBOX_MAPS.to64[sq(File::E, Rank::R4)];

    uint64_t white_pawns = pos.get_white_pawns();
    EXPECT_FALSE(getBit(white_pawns, e2_sq64)) << "White pawn should no longer be on e2";
    EXPECT_TRUE(getBit(white_pawns, e4_sq64)) << "White pawn should now be on e4";
    EXPECT_EQ(total_count(pos, PieceType::Pawn), initial_pawn_count) << "Pawn count unchanged";
    EXPECT_EQ(pos.king_sq, initial_king_sq) << "King positions should be unchanged";

    pos.TakeMove();

    EXPECT_EQ(pos.king_sq, initial_king_sq) << "King positions should be restored";
    EXPECT_EQ(pos.get_white_pawns(), initial_white_pawns) << "White pawn bitboard restored";
    EXPECT_EQ(total_count(pos, PieceType::Pawn), initial_pawn_count) << "Pawn count restored";

    EXPECT_EQ(pos.at_sq64(move.get_from()), make_piece(Color::White, PieceType::Pawn));
    EXPECT_EQ(pos.at_sq64(move.get_to()), Piece::None);
}

TEST_F(IncrementalUpdateTest, CaptureMoveMaintainsCorrectCounts) {
    // Place a black pawn on e4 that can be captured
    pos.set(sq(File::E, Rank::R4), make_piece(Color::Black, PieceType::Pawn));
    pos.rebuild_counts();

    int initial_pawn_count = total_count(pos, PieceType::Pawn);
    uint64_t initial_white_pawns = pos.get_white_pawns();

    // Make a capture move: d2xe4
    S_MOVE move = make_capture(sq64(File::D, Rank::R2), sq64(File::E, Rank::R4), PieceType::Pawn);
    int move_result = pos.MakeMove(move);
    EXPECT_EQ(move_result, 1) << "Move should be legal";

    EXPECT_EQ(total_count(pos, PieceType::Pawn), initial_pawn_count - 1)
        << "Pawn count should decrease by 1 due to capture";

    int e4_sq64 = MAILBOX_MAPS.to64[sq(File::E, Rank::R4)];
    EXPECT_TRUE(getBit(pos.get_white_pawns(), e4_sq64)) << "White pawn should be on e4";

    pos.TakeMove();

    EXPECT_EQ(total_count(pos, PieceType::Pawn), initial_pawn_count) << "Pawn count restored";
    EXPECT_EQ(pos.get_white_pawns(), initial_white_pawns) << "White pawn bitboard restored";
    EXPECT_EQ(pos.at(sq(File::E, Rank::R4)), make_piece(Color::Black, PieceType::Pawn));
    EXPECT_EQ(pos.at(sq(File::D, Rank::R2)), make_piece(Color::White, PieceType::Pawn));
}

TEST_F(IncrementalUpdateTest, KingMoveMaintainsKingSquare) {
    pos.set(sq(File::E, Rank::R2), Piece::None);
    pos.rebuild_counts();

    auto initial_king_sq = pos.king_sq;

    S_MOVE move = make_move(sq64(File::E, Rank::R1), sq64(File::E, Rank::R2));
    int move_result = pos.MakeMove(move);
    EXPECT_EQ(move_result, 1) << "King move should be legal";

    EXPECT_EQ(pos.king_sq[size_t(Color::White)], sq64(File::E, Rank::R2));
    EXPECT_EQ(pos.king_sq[size_t(Color::Black)], initial_king_sq[size_t(Color::Black)]);

    pos.TakeMove();
    EXPECT_EQ(pos.king_sq, initial_king_sq) << "King squares should be restored";
}
