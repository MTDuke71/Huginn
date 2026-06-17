#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/search.hpp"

namespace {

int eval_white_pov(const std::string& fen) {
    Position pos;
    EXPECT_TRUE(pos.set_from_fen(fen));
    Huginn::Engine engine;
    return engine.evalPosition(pos);
}

}  // namespace

TEST(EvalOutposts, SupportedKnightHoleScoresBetterThanChallengeableSquare) {
    Huginn::init();

    // White Nd5 is supported by c4. In the first position Black's same-material
    // pawn sits on a6 and cannot ever challenge d5; in the second it sits on e6,
    // attacking the knight immediately and invalidating the outpost.
    const int true_outpost = eval_white_pov("6k1/8/p7/3N4/2P5/8/8/6K1 w - - 0 1");
    const int challenged = eval_white_pov("6k1/8/4p3/3N4/2P5/8/8/6K1 w - - 0 1");

    EXPECT_GT(true_outpost, challenged)
        << "A supported knight hole should score above the same material with "
           "an enemy pawn able to challenge it.";
}

TEST(EvalOutposts, SupportedBishopHoleScoresBetterThanChallengeableSquare) {
    Huginn::init();

    // Same outpost geometry as the knight case, but with Bd5. This keeps the
    // bishop-specific bonus covered as its own tunable term.
    const int true_outpost = eval_white_pov("6k1/8/p7/3B4/2P5/8/8/6K1 w - - 0 1");
    const int challenged = eval_white_pov("6k1/8/4p3/3B4/2P5/8/8/6K1 w - - 0 1");

    EXPECT_GT(true_outpost, challenged)
        << "A supported bishop hole should score above the same material with "
           "an enemy pawn able to challenge it.";
}

TEST(EvalOutposts, MirroredKnightOutpostsAreColorSymmetric) {
    Huginn::init();

    const int white_outpost = eval_white_pov("6k1/8/p7/3N4/2P5/8/8/6K1 w - - 0 1");
    const int black_outpost_from_black_pov =
        eval_white_pov("6k1/8/8/2p5/3n4/P7/8/6K1 b - - 0 1");

    EXPECT_EQ(white_outpost, black_outpost_from_black_pov)
        << "Mirrored outposts should receive the same side-to-move score.";
}
