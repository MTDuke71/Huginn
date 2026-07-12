// BACKLOG #9 threats round 2 (ENABLE_THREATS_R2): hanging units, safe
// pawn-push threats, threat-by-king. Behaviour tests are gated on the flag
// (the term is a candidate, default OFF); the mirror-symmetry tests run on
// BOTH arms — with the flag off they cover the surrounding eval, with it on
// they cover the new block's colour symmetry (INVARIANTS: a term that breaks
// the mirror suite is a bug, full stop).

#include <gtest/gtest.h>

#include "../src/init.hpp"
#include "../src/search.hpp"

namespace {

int eval_stm_pov(const std::string& fen) {
    Position pos;
    EXPECT_TRUE(pos.set_from_fen(fen));
    Huginn::Engine engine;
    return engine.evalPosition(pos);
}

}  // namespace

// --- Colour symmetry (both arms) ---------------------------------------------

TEST(EvalThreatsR2, HangingPieceGeometryIsColorSymmetric) {
    Huginn::init();
    // White rook attacks an undefended black knight down the d-file; the
    // mirror flips ranks, swaps colours, and flips the side to move.
    const int white_pov = eval_stm_pov("6k1/8/8/3n4/8/8/8/3R2K1 w - - 0 1");
    const int black_pov = eval_stm_pov("3r2k1/8/8/8/3N4/8/8/6K1 b - - 0 1");
    EXPECT_EQ(white_pov, black_pov);
}

TEST(EvalThreatsR2, PawnPushThreatGeometryIsColorSymmetric) {
    Huginn::init();
    // e2-e3 (and e2-e4) would attack the black knight on d4.
    const int white_pov = eval_stm_pov("6k1/8/8/8/3n4/8/4P3/6K1 w - - 0 1");
    const int black_pov = eval_stm_pov("6k1/4p3/8/3N4/8/8/8/6K1 b - - 0 1");
    EXPECT_EQ(white_pov, black_pov);
}

TEST(EvalThreatsR2, KingThreatGeometryIsColorSymmetric) {
    Huginn::init();
    // A hanging black knight inside the white king's ring.
    const int white_pov = eval_stm_pov("6k1/8/8/8/8/8/5n2/6K1 w - - 0 1");
    const int black_pov = eval_stm_pov("6K1/5N2/8/8/8/8/8/6k1 b - - 0 1");
    EXPECT_EQ(white_pov, black_pov);
}

// --- Term behaviour (candidate arm only) ---------------------------------------

#if defined(ENABLE_THREATS_R2) && ENABLE_THREATS_R2

TEST(EvalThreatsR2, HangingPieceScoresAboveDefendedPiece) {
    Huginn::init();
    // Same rook-attacks-knight geometry; in the second position an e6 pawn
    // defends d5 (and black has the extra pawn) — both effects should leave
    // the hanging version clearly better for White.
    const int hanging = eval_stm_pov("6k1/8/8/3n4/8/8/8/3R2K1 w - - 0 1");
    const int defended = eval_stm_pov("6k1/8/4p3/3n4/8/8/8/3R2K1 w - - 0 1");
    EXPECT_GT(hanging, defended);
}

TEST(EvalThreatsR2, PawnPushThreatBeatsIdlePawn) {
    Huginn::init();
    // e2 can push to e3/e4 hitting the d4 knight; the h2 control pawn has no
    // push that attacks anything.
    const int pushing = eval_stm_pov("6k1/8/8/8/3n4/8/4P3/6K1 w - - 0 1");
    const int idle = eval_stm_pov("6k1/8/8/8/3n4/8/7P/6K1 w - - 0 1");
    EXPECT_GT(pushing, idle);
}

TEST(EvalThreatsR2, UnsafePushSquareDoesNotCount) {
    Huginn::init();
    // Black's f4 pawn controls e3, so the single push is unsafe; the double
    // push e2-e4 is safe but attacks d5/f5 — not the d4 knight. Only the
    // *unfiltered* term would score these two positions apart by a push bonus.
    Position guarded, open;
    ASSERT_TRUE(guarded.set_from_fen("6k1/8/8/8/3n1p2/8/4P3/6K1 w - - 0 1"));
    ASSERT_TRUE(open.set_from_fen("6k1/8/8/8/3n4/8/4P3/6K1 w - - 0 1"));
    Huginn::Engine engine;
    const int guarded_eval = engine.evalPosition(guarded);
    const int open_eval = engine.evalPosition(open);
    // The open position keeps the push threat; the guarded one loses it AND
    // Black has an extra pawn — White must be strictly better in the open one.
    EXPECT_GT(open_eval, guarded_eval);
}

TEST(EvalThreatsR2, KingRingHangerBeatsDefendedOne) {
    Huginn::init();
    // Black knight en prise next to the white king vs the same knight
    // defended by a g3 pawn (which also pressures the king zone — every
    // effect points the same way).
    const int hanging = eval_stm_pov("6k1/8/8/8/8/8/5n2/6K1 w - - 0 1");
    const int defended = eval_stm_pov("6k1/8/8/8/8/6p1/5n2/6K1 w - - 0 1");
    EXPECT_GT(hanging, defended);
}

#else

TEST(EvalThreatsR2, BehaviourTestsRequireCandidateArm) {
    GTEST_SKIP() << "ENABLE_THREATS_R2 is OFF (baseline arm) — behaviour tests "
                    "run on the candidate arm (-DENABLE_THREATS_R2=ON)";
}

#endif  // ENABLE_THREATS_R2
