#include <gtest/gtest.h>

#include <cstdlib>

#include "../src/init.hpp"
#include "../src/search.hpp"

// ENABLE_DRAWISHNESS_SCALING lives in src/search.cpp (#9/#35 "mul[]"). When
// CMake forwards -DENABLE_DRAWISHNESS_SCALING=0/1 the definition is global and
// visible here; unset means the in-source default — keep this mirror of the
// default in sync with search.cpp so the scaled-magnitude assertions below are
// compiled only on the arm that actually scales.
#ifndef ENABLE_DRAWISHNESS_SCALING
#define ENABLE_DRAWISHNESS_SCALING 1
#endif

namespace {

int eval_stm(const std::string& fen) {
    Position pos;
    EXPECT_TRUE(pos.set_from_fen(fen));
    Huginn::Engine engine;
    return engine.evalPosition(pos);
}

// White Kg1 Bd3(light) Pa4,b4,c4 vs Black Kg8 Be5(dark) Ph7 — pure
// opposite-coloured bishops, White two healthy pawns up. The canonical
// "winning on material, dead drawn on the board" shape rule 1 halves.
const char* kPureOcbFen = "6k1/7p/8/4b3/PPP5/3B4/8/6K1 w - - 0 1";

// Same position with the black bishop on g6 (light — SAME colour as Bd3):
// identical material, no OCB, so the score must be unscaled.
const char* kSameColourBishopsFen = "6k1/7p/6b1/8/PPP5/3B4/8/6K1 w - - 0 1";

}  // namespace

#if ENABLE_DRAWISHNESS_SCALING
TEST(EvalDrawishness, PureOcbIsRoughlyHalvedVsSameColourBishops) {
    Huginn::init();

    const int ocb = eval_stm(kPureOcbFen);
    const int same_colour = eval_stm(kSameColourBishopsFen);

    // Both are White +2 pawns and must still favour White.
    EXPECT_GT(ocb, 0);
    EXPECT_GT(same_colour, 0);

    // The unscaled evals of the two positions differ only by bishop PST /
    // mobility noise, so the 64/128 factor must pull the OCB score well below
    // the same-colour analog — roughly half, asserted as a generous band
    // rather than an exact cp value.
    EXPECT_LT(ocb, same_colour * 3 / 4)
        << "Pure OCB should be scaled to ~half of the same-colour-bishops analog.";
    EXPECT_GT(ocb, same_colour / 4)
        << "OCB is halved (64/128), not slashed to the 16/128 pawnless factor.";
}

TEST(EvalDrawishness, PawnlessMinorAdvantageIsNearDraw) {
    Huginn::init();

    // KRB vs KR, no pawns anywhere: White is a full bishop up but cannot
    // normally win — rule 2 (favoured side pawnless, edge <= one minor)
    // scales the score to 16/128.
    const int eval = eval_stm("1r4k1/8/8/8/8/3B4/8/R5K1 w - - 0 1");

    EXPECT_GT(eval, 0) << "White is still the favoured side.";
    EXPECT_LT(eval, 120)
        << "A pawnless one-minor edge must be scaled toward a draw, not scored "
           "as a won bishop.";
}
#endif  // ENABLE_DRAWISHNESS_SCALING

TEST(EvalDrawishness, OcbScalingIsColourSymmetric) {
    Huginn::init();

    // Hand-mirrored kPureOcbFen (ranks flipped, colours swapped, Black to
    // move). Side-to-move POV scores must match exactly — including the
    // toward-zero truncation of score*mul/128 (INVARIANTS.md colour symmetry).
    const int white_pov = eval_stm(kPureOcbFen);
    const int black_pov = eval_stm("6k1/8/3b4/ppp5/4B3/8/7P/6K1 b - - 0 1");

    EXPECT_EQ(white_pov, black_pov)
        << "evaluate(pos) == -evaluate(mirror(pos)) must hold under scaling.";
}

TEST(EvalDrawishness, RookEndgameStaysUnscaled) {
    Huginn::init();

    // Same shape as the OCB pair but with rooks: White two pawns up in a rook
    // endgame is a genuine winning edge and must NOT be damped (rooks disable
    // rule 1; the favoured side has pawns, disabling rule 2). Holds on both
    // arms — the threshold is far below the unscaled score and far above a
    // 64/128- or 16/128-scaled one.
    const int eval = eval_stm("6k1/7p/8/4r3/PPP5/3R4/8/6K1 w - - 0 1");

    EXPECT_GT(eval, 100)
        << "A two-pawns-up rook endgame must keep its full winning score.";
}
