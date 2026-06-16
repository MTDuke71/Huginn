#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/position.hpp"

#include <string>

TEST(PositionIntegrity, AcceptsNormalMakeUnmakeSequence) {
    Huginn::init();

    Position pos;
    pos.set_startpos();

    std::string reason;
    ASSERT_TRUE(pos.is_consistent(&reason)) << reason;

    const S_MOVE move = make_move(sq64(File::E, Rank::R2), sq64(File::E, Rank::R4));
    ASSERT_EQ(pos.MakeMove(move), 1);
    EXPECT_TRUE(pos.is_consistent(&reason)) << reason;

    pos.TakeMove();
    EXPECT_TRUE(pos.is_consistent(&reason)) << reason;
}

TEST(PositionIntegrity, RejectsCorruptDerivedCachesAndZobrist) {
    Huginn::init();

    Position pos;
    pos.set_startpos();

    std::string reason;
    Position bad_occupied = pos;
    bad_occupied.occupied_bitboard ^= (1ULL << sq64(File::A, Rank::R3));
    EXPECT_FALSE(bad_occupied.is_consistent(&reason));

    Position bad_king = pos;
    bad_king.king_sq[int(Color::White)] = sq64(File::D, Rank::R1);
    EXPECT_FALSE(bad_king.is_consistent(&reason));

    Position bad_zobrist = pos;
    bad_zobrist.zobrist_key ^= 1ULL;
    EXPECT_FALSE(bad_zobrist.is_consistent(&reason));
}
