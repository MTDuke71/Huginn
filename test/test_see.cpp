/**
 * @file test_see.cpp
 * @brief Sanity tests for Static Exchange Evaluation (see.hpp).
 */

#include <gtest/gtest.h>

#include "../src/see.hpp"
#include "../src/position.hpp"
#include "../src/movegen.hpp"
#include "../src/init.hpp"

using namespace Huginn;

namespace {

// Find a move from -> to (in algebraic, e.g. "e4", "d5") in the legal move
// list of `pos`. Returns the matching S_MOVE, or .move=0 if not found.
S_MOVE find_move(const Position& pos, const std::string& from_alg, const std::string& to_alg) {
    int from120 = (from_alg[0] - 'a') + (from_alg[1] - '1') * 10 + 21;
    int to120   = (to_alg[0]   - 'a') + (to_alg[1]   - '1') * 10 + 21;
    S_MOVELIST list;
    generate_legal_moves(const_cast<Position&>(pos), list);
    for (int i = 0; i < list.count; ++i) {
        if (list.moves[i].get_from() == from120 && list.moves[i].get_to() == to120) {
            return list.moves[i];
        }
    }
    S_MOVE none{};
    none.move = 0;
    return none;
}

class SEETest : public ::testing::Test {
protected:
    void SetUp() override {
        // Make sure all the bitboard / attack-table init has happened.
        Huginn::init();
    }
};

// White pawn captures undefended black pawn → SEE = +pawn.
TEST_F(SEETest, UndefendedPawnCapture) {
    Position pos;
    pos.set_from_fen("4k3/8/8/4p3/3P4/8/8/4K3 w - - 0 1");
    auto m = find_move(pos, "d4", "e5");
    ASSERT_NE(m.move, 0);
    EXPECT_EQ(see(pos, m), 100);
}

// Bishop trades for bishop, defended → SEE = 0 (equal trade).
// White Be4 takes Black Bd5; Black Pc6 recaptures.
TEST_F(SEETest, EqualBishopTrade) {
    Position pos;
    pos.set_from_fen("4k3/8/2p5/3b4/4B3/8/8/4K3 w - - 0 1");
    auto m = find_move(pos, "e4", "d5");
    ASSERT_NE(m.move, 0);
    EXPECT_EQ(see(pos, m), 0);
}

// Queen takes defended pawn → gain pawn, lose queen → SEE = 100 - 900 = -800.
// Black pawn on f6 defends e5; white queen on e1 captures e5.
TEST_F(SEETest, LosingQueenForPawn) {
    Position pos;
    pos.set_from_fen("4k3/8/5p2/4p3/8/8/8/4Q1K1 w - - 0 1");
    auto m = find_move(pos, "e1", "e5");
    ASSERT_NE(m.move, 0);
    EXPECT_EQ(see(pos, m), 100 - 900);
}

// Pawn captures knight defended by queen → SEE = N - P = 320 - 100 = 220.
// (Original test had bq on d2 giving check to ke1 — illegal-move filter
// hid the capture. Move the bq to a8 where it doesn't check.)
TEST_F(SEETest, PawnTakesDefendedKnight) {
    Position pos;
    pos.set_from_fen("q3k3/8/8/3n4/2P5/8/8/4K3 w - - 0 1");
    auto m = find_move(pos, "c4", "d5");
    ASSERT_NE(m.move, 0);
    EXPECT_EQ(see(pos, m), 320 - 100);
}

// X-ray attacker through a removed slider:
// White rook captures black pawn defended only by black bishop. After RxP,
// BxR, white rook on a1 is x-ray attacker but only after the first rook
// is gone. Or simpler: W rook on a1 + W rook on a3, B pawn on a5, B rook on a8.
// 1) RxP (a3-a5)  gain pawn
// 2) RxR  (a8-a5) lose rook (-500)  → -400
// 3) RxR  (a1-a5) gain rook (+500)  → +100
// SEE = 100. Tests that the second white rook is found via x-ray after
// removing the first.
TEST_F(SEETest, XRayThroughRook) {
    Position pos;
    pos.set_from_fen("r3k3/8/8/p7/8/R7/8/R3K3 w - - 0 1");
    auto m = find_move(pos, "a3", "a5");
    ASSERT_NE(m.move, 0);
    EXPECT_EQ(see(pos, m), 100);
}

} // namespace
