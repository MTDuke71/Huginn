#include <gtest/gtest.h>
#include "position.hpp"
#include "movegen.hpp"

// Tiny perft harness (uses legal moves; grow as you add rules)
static uint64_t perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    MoveList list; generate_legal_moves(pos, list);
    uint64_t nodes = 0;
    for (const auto& m : list.v) {
        State st{};
        make_move(pos, m, st);
        nodes += perft(pos, depth-1);
        unmake_move(pos, m, st);
    }
    return nodes;
}

// Smoke: start position is initialized correctly
TEST(Perft, StartposSmoke_KingsAndPawns) {
    Position pos; pos.set_startpos();
    int whitePawns=0, blackPawns=0, whiteKings=0, blackKings=0;
    for (int r=0; r<8; ++r) {
        for (int f=0; f<8; ++f) {
            int s = sq(static_cast<File>(f), static_cast<Rank>(r));
            Piece p = pos.at(s);
            if (is_none(p)) continue;
            if (p == Piece::WhitePawn) ++whitePawns;
            if (p == Piece::BlackPawn) ++blackPawns;
            if (p == Piece::WhiteKing) ++whiteKings;
            if (p == Piece::BlackKing) ++blackKings;
        }
    }
    EXPECT_EQ(whitePawns, 8);
    EXPECT_EQ(blackPawns, 8);
    EXPECT_EQ(whiteKings, 1);
    EXPECT_EQ(blackKings, 1);
}

// These are disabled until your generator covers all piece types & rules.
// Enable one depth at a time as you pass prior tests.

TEST(Perft, DISABLED_Startpos_d1_is_20) {
    Position pos; pos.set_startpos();
    EXPECT_EQ(perft(pos, 1), 20u);
}

TEST(Perft, DISABLED_Startpos_d2_is_400) {
    Position pos; pos.set_startpos();
    EXPECT_EQ(perft(pos, 2), 400u);
}

TEST(Perft, DISABLED_Startpos_d3_is_8902) {
    Position pos; pos.set_startpos();
    EXPECT_EQ(perft(pos, 3), 8902u);
}

// Kiwipete covers castling, pins, etc. Enable after specials work.
TEST(Perft, DISABLED_Kiwipete_d1_48_d2_2039) {
    Position pos; pos.clear();
    // Set up Kiwipete by hand or add a small FEN parser later.
    // For now, leave disabled until FEN parsing + full move-gen exist.
    SUCCEED();
}
