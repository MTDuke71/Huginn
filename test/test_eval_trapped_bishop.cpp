#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/search.hpp"

// BACKLOG #20 (ENABLE_TRAPPED_BISHOP): a bishop cornered behind an enemy pawn
// (Ba7/pb6 and the other CPW EvalBishop locks) is usually lost for that pawn.
// These tests exercise the flag-ON arm — the branch default.

namespace {

int eval_stm_pov(const std::string& fen) {
    Position pos;
    EXPECT_TRUE(pos.set_from_fen(fen));
    Huginn::Engine engine;
    return engine.evalPosition(pos);
}

}  // namespace

TEST(EvalTrappedBishop, BishopA7BehindB6PawnScoresWorseThanNonTrappingPawn) {
    Huginn::init();

    // Same material either way; only the black pawn moves b6 -> b7. On b6 it
    // locks the a7 bishop in the corner; on b7 the bishop is free. The penalty
    // (100 MG / 120 EG) must dominate the small pawn PST / passed-rank drift.
    const int trapped = eval_stm_pov("6k1/B7/1p6/8/8/8/8/6K1 w - - 0 1");
    const int free_bishop = eval_stm_pov("6k1/Bp6/8/8/8/8/8/6K1 w - - 0 1");

    EXPECT_LT(trapped, free_bishop - 50)
        << "A bishop locked on a7 by a b6 pawn should score measurably below "
           "the same material with the pawn on b7.";
}

TEST(EvalTrappedBishop, MirroredTrappedBishopsAreColorSymmetric) {
    Huginn::init();

    // Colour mirror of Ba7/pb6 is Ba2/Pb3 with Black to move; the
    // side-to-move scores must match exactly (INVARIANTS colour symmetry).
    const int white_trapped = eval_stm_pov("6k1/B7/1p6/8/8/8/8/6K1 w - - 0 1");
    const int black_trapped_from_black_pov =
        eval_stm_pov("6k1/8/8/8/8/1P6/b7/6K1 b - - 0 1");

    EXPECT_EQ(white_trapped, black_trapped_from_black_pov)
        << "Mirrored trapped bishops should receive the same side-to-move score.";
}

TEST(EvalTrappedBishop, A6LockIsLighterThanA7Lock) {
    Huginn::init();

    // Penalty deltas (non-trapping control minus trapped) per tier: the full
    // a7 corner lock must cost more than the lighter a6 one-step lock.
    const int a7_trapped = eval_stm_pov("6k1/B7/1p6/8/8/8/8/6K1 w - - 0 1");
    const int a7_control = eval_stm_pov("6k1/Bp6/8/8/8/8/8/6K1 w - - 0 1");
    const int a6_trapped = eval_stm_pov("6k1/8/B7/1p6/8/8/8/6K1 w - - 0 1");
    const int a6_control = eval_stm_pov("6k1/8/Bp6/8/8/8/8/6K1 w - - 0 1");

    EXPECT_GT(a7_control - a7_trapped, a6_control - a6_trapped)
        << "The a7-tier corner lock should be penalized harder than the "
           "a6-tier one-step lock.";
}

TEST(EvalTrappedBishop, NoPenaltyWhenTrappingPawnAbsent) {
    Huginn::init();

    // With the black pawn on b5 instead of b6, no lock fires on the a7 bishop
    // (b5 only locks a bishop on a6), so the eval gap to the trapped position
    // must be roughly the full penalty — not zero.
    const int trapped = eval_stm_pov("6k1/B7/1p6/8/8/8/8/6K1 w - - 0 1");
    const int pawn_off_lock = eval_stm_pov("6k1/B7/8/1p6/8/8/8/6K1 w - - 0 1");

    EXPECT_GT(pawn_off_lock, trapped + 50)
        << "The penalty must be tied to the b6 lock pawn, not to the bishop "
           "square alone.";
}
