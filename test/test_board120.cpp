#include <gtest/gtest.h>
#include "board120.hpp"    // core: File, Rank, sq(), is_playable, file_of, rank_of, deltas, MAILBOX_MAPS
#include "squares120.hpp"  // extras: Square120::E4, Playable120 (no redefs)
#include "position.hpp"

TEST(Board120, SqCalculation) {
    EXPECT_EQ(sq(File::A, Rank::R1), 21);
    EXPECT_EQ(sq(File::H, Rank::R1), 28);
    EXPECT_EQ(sq(File::A, Rank::R8), 91);
    EXPECT_EQ(sq(File::H, Rank::R8), 98);
    EXPECT_EQ(Square120::E4, sq(File::E, Rank::R4));  // named constant aligns
}

TEST(Board120, PlayableAndOffboardFrames) {
    EXPECT_TRUE(is_playable(21));  // A1
    EXPECT_TRUE(is_playable(98));  // H8
    EXPECT_FALSE(is_playable(20)); // left frame
    EXPECT_FALSE(is_playable(29)); // right frame
    EXPECT_FALSE(is_playable(11)); // bottom frame
    EXPECT_FALSE(is_playable(109));// top frame
}

TEST(Board120, FileRankOfRoundTrip) {
    for (int r = 0; r < 8; ++r)
      for (int f = 0; f < 8; ++f) {
        const auto F = static_cast<File>(f);
        const auto R = static_cast<Rank>(r);
        const int s  = sq(F, R);
        ASSERT_TRUE(is_playable(s));
        EXPECT_EQ(file_of(s), F);
        EXPECT_EQ(rank_of(s), R);
      }
}

TEST(Board120, DirectionOffsets) {
    const int e2 = sq(File::E, Rank::R2);
    EXPECT_EQ(e2 + NORTH, sq(File::E, Rank::R3));
    EXPECT_EQ(e2 + SOUTH, sq(File::E, Rank::R1));
    EXPECT_EQ(e2 + EAST , sq(File::F, Rank::R2));
    EXPECT_EQ(e2 + WEST , sq(File::D, Rank::R2));
    EXPECT_EQ(e2 + NE   , sq(File::F, Rank::R3));
    EXPECT_EQ(e2 + NW   , sq(File::D, Rank::R3));
    EXPECT_EQ(e2 + SE   , sq(File::F, Rank::R1));
    EXPECT_EQ(e2 + SW   , sq(File::D, Rank::R1));
}

TEST(Board120, KnightTargetsFromCorners) {
    // From A1 (21): only C2 (sq(File::C,R2)=23+? actually 21+12=33) and B3 (21+?=40) are playable
    const int a1 = sq(File::A, Rank::R1);
    int playable = 0;
    for (int d : KNIGHT_DELTAS) {
        const int to = a1 + d;
        if (is_playable(to)) ++playable;
    }
    EXPECT_EQ(playable, 2);

    // From G1: E2, F3, H3 are playable => 3
    const int g1 = sq(File::G, Rank::R1);
    playable = 0;
    for (int d : KNIGHT_DELTAS) if (is_playable(g1 + d)) ++playable;
    EXPECT_EQ(playable, 3);
}

TEST(Board120, KingNeighborhoodCenterVsEdge) {
    const int d4 = sq(File::D, Rank::R4);
    int neighbors = 0;
    for (int d : KING_DELTAS) if (is_playable(d4 + d)) ++neighbors;
    EXPECT_EQ(neighbors, 8);

    const int a1 = sq(File::A, Rank::R1);
    neighbors = 0;
    for (int d : KING_DELTAS) if (is_playable(a1 + d)) ++neighbors;
    EXPECT_EQ(neighbors, 3);
}

TEST(Board120, MailboxMapsRoundTrip64) {
    for (int s64 = 0; s64 < 64; ++s64) {
        const int s120 = MAILBOX_MAPS.to120[size_t(s64)];
        ASSERT_TRUE(is_playable(s120));
        const int back = MAILBOX_MAPS.to64[size_t(s120)];
        EXPECT_EQ(back, s64);
    }
    // check a couple offboard cells
    EXPECT_EQ(MAILBOX_MAPS.to64[0], -1);
    EXPECT_EQ(MAILBOX_MAPS.to64[119], -1);
}

TEST(Board120, IteratePlayable120) {
    int count = 0;
    for (int s : Playable120{}) {
        ASSERT_TRUE(is_playable(s));
        ++count;
    }
    EXPECT_EQ(count, 64);
}

// Helper for mailbox-120 raw indexing:
// file = 1..8 are A..H playable files; 9 is the right border (I),
// row  = 0..11 are the 12 rows; playable rows are 2..9.
static constexpr int raw120(int file1based, int row) {
    return row * 10 + file1based;
}

TEST(Board120, ExplicitOffboardSentinels) {
    // A0  -> bottom outer frame (row 0, file 1)
    EXPECT_FALSE(is_playable(raw120(1, 0)));

    // A9  -> top outer frame (row 10, file 1). (Playable top is row 9.)
    EXPECT_FALSE(is_playable(raw120(1, 10)));

    // I1  -> right outer frame next to H1 (file 9, playable rows start at 2)
    EXPECT_FALSE(is_playable(raw120(9, 2)));

    // And a couple of sanity checks that real squares ARE playable:
    EXPECT_TRUE(is_playable(sq(File::A, Rank::R1))); // 21
    EXPECT_TRUE(is_playable(sq(File::H, Rank::R8))); // 98
}

TEST(Board120, BoardClearSetsFramesOffboard) {
    Position b; b.clear();
    auto raw120 = [](int file1, int row){ return row*10 + file1; };
    // Offboard squares should be Piece::None
    EXPECT_EQ(b.at(raw120(1,0)), Piece::None);  // A0
    EXPECT_EQ(b.at(raw120(9,2)), Piece::None);  // I1 (right frame)
    EXPECT_EQ(b.at(raw120(1,10)), Piece::None); // A10 (top frame)
    // E4 playable = Piece::None after clear
    EXPECT_EQ(b.at(sq(File::E, Rank::R4)), Piece::None);
}