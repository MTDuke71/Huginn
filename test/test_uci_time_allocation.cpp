// BACKLOG #56 (part 3): clock math at the UCI boundary.
//
// compute_time_budget_ms is the pure, 64-bit per-move allocator. The
// allocation STRATEGY is the gauntleted #47 tuning (time/20 + inc/2 sudden
// death, time/movestogo + inc/2 classical, reserve cap, 60% cap) — these
// tests pin the boundary behaviour the old inline int math got wrong: the
// final max(50, alloc) floor budgeted 50 ms with 1-10 ms on the clock
// (overdrawing the reserve it had just computed), and unchecked int
// arithmetic could overflow on hostile inputs. Also covers the strict
// go-parameter parser (parse_spin_clamped) now shared with setoption.

#include "gtest/gtest.h"
#include "../src/uci_utils.hpp"
#include "../src/uci.hpp"
#include "../src/init.hpp"
#include <algorithm>
#include <chrono>
#include <sstream>
#include <vector>

namespace {

// The reserve the allocator holds back, mirrored for invariant checks.
long long reserve_of(long long time_ms) {
    return std::max(50LL, std::min(1000LL, time_ms / 10));
}

} // namespace

// --- The acceptance-criteria boundary cases: 0/1/10/49/50/100 ms -------------

TEST(UciTimeAllocation, BoundaryCasesNeverOverdrawTinyClocks) {
    // At or below the 50 ms reserve the safely usable remainder is zero: the
    // budget collapses to the 1 ms emergency move (the old code returned
    // 50 ms for every one of these — a forfeit with 1-10 ms on the clock).
    EXPECT_EQ(compute_time_budget_ms(0, 0, 0), 1);
    EXPECT_EQ(compute_time_budget_ms(1, 0, 0), 1);
    EXPECT_EQ(compute_time_budget_ms(10, 0, 0), 1);
    EXPECT_EQ(compute_time_budget_ms(49, 0, 0), 1);
    EXPECT_EQ(compute_time_budget_ms(50, 0, 0), 1);

    // 100 ms clock: reserve 50, safely usable 50 — the quality floor may use
    // all of it but not a millisecond more.
    EXPECT_EQ(compute_time_budget_ms(100, 0, 0), 50);

    // An increment does not license overdrawing the *current* clock — the
    // increment only lands after the move is completed.
    EXPECT_EQ(compute_time_budget_ms(10, 1000, 0), 1);
    EXPECT_EQ(compute_time_budget_ms(100, 1000, 0), 50);
}

// --- Invariants across the input grid -----------------------------------------

TEST(UciTimeAllocation, BudgetAlwaysPositiveAndWithinSafeRemainder) {
    const long long times[] = {0, 1, 10, 49, 50, 51, 60, 100, 200, 500, 1000,
                               5000, 60000, 300000, 1000000000LL};
    const long long incs[] = {0, 10, 100, 1000, 1000000000LL};
    const long long mtgs[] = {0, 1, 2, 30, 500};

    for (long long t : times) {
        for (long long inc : incs) {
            for (long long mtg : mtgs) {
                const long long budget = compute_time_budget_ms(t, inc, mtg);
                ASSERT_GE(budget, 1) << "t=" << t << " inc=" << inc << " mtg=" << mtg;
                // Never beyond the safely usable remainder (modulo the 1 ms
                // emergency minimum when the whole clock is inside the reserve).
                const long long safe_max = std::max(0LL, t - reserve_of(t));
                ASSERT_LE(budget, std::max(1LL, safe_max))
                    << "t=" << t << " inc=" << inc << " mtg=" << mtg;
            }
        }
    }
}

TEST(UciTimeAllocation, MovestogoOneDoesNotSpendTheWholeClock) {
    // Last move of the period: the naive time/1 allocation must still be
    // capped by the reserve and the 60% ceiling.
    const long long budget = compute_time_budget_ms(60000, 0, 1);
    EXPECT_LE(budget, 36000);  // 60% cap
    EXPECT_GE(budget, 1000);   // but it does use a large share
}

TEST(UciTimeAllocation, NormalClocksMatchTheShippedFormula) {
    // 60s + 0.1s inc sudden death (the house gauntlet TC at move ~1):
    // time/20 + inc/2 = 3000 + 50 — the #47 strategy, unchanged.
    EXPECT_EQ(compute_time_budget_ms(60000, 100, 0), 3050);
    // Classical 40/5m: 300000/40 + 0 = 7500.
    EXPECT_EQ(compute_time_budget_ms(300000, 0, 40), 7500);
}

TEST(UciTimeAllocation, HostileMagnitudesDoNotOverflow) {
    // Saturation bound (1e9, the parse cap) in every slot: all internal
    // sums stay far inside int64 and the result respects the caps.
    const long long budget = compute_time_budget_ms(1000000000LL, 1000000000LL, 1);
    EXPECT_GT(budget, 0);
    EXPECT_LE(budget, 600000000LL);  // 60% cap
}

TEST(UciTimeAllocation, UnknownClockFallsBackToIncrementSlice) {
    // Our clock not sent (negative = unset): a conservative inc/4 slice,
    // floored at 50 ms; with no increment either, the bare-go 5s default.
    EXPECT_EQ(compute_time_budget_ms(-1, 2000, 0), 500);
    EXPECT_EQ(compute_time_budget_ms(-1, 40, 0), 50);
    EXPECT_EQ(compute_time_budget_ms(-1, 0, 0), 5000);
}

// --- BACKLOG #65: hard (extension) deadline -----------------------------------

TEST(UciHardTimeBudget, IsThreeTimesSoftOnNormalClocks) {
    // The house gauntlet TC. Soft is unchanged at time/20 + inc/2 = 550; the
    // hard bound is 3x that, well inside both caps (safe_max 9000, cap60 6000).
    EXPECT_EQ(compute_time_budget_ms(10000, 100, 0), 550);
    EXPECT_EQ(compute_hard_time_budget_ms(10000, 100, 0), 1650);
    // 60+0.1: soft 3050, hard 9150 (safe_max 59000, cap60 36000 — neither bites).
    EXPECT_EQ(compute_hard_time_budget_ms(60000, 100, 0), 9150);
}

TEST(UciHardTimeBudget, NeverExceedsTheAntiForfeitCaps) {
    // #47's two caps still bind the extension. Sweep a wide grid and assert the
    // hard deadline can never reach into the reserve or past 60% of the clock —
    // this is the property that makes #65 safe by construction.
    for (long long t : {1LL, 10LL, 50LL, 100LL, 500LL, 1000LL, 10000LL, 60000LL, 300000LL}) {
        for (long long inc : {0LL, 50LL, 100LL, 1000LL}) {
            for (long long mtg : {0LL, 1LL, 5LL, 40LL}) {
                const long long soft = compute_time_budget_ms(t, inc, mtg);
                const long long hard = compute_hard_time_budget_ms(t, inc, mtg);
                const long long reserve = std::max(50LL, std::min(1000LL, t / 10));
                EXPECT_GE(hard, soft) << "hard must never shorten a search";
                EXPECT_GE(hard, 1);
                // The allowance may not exceed what the soft path already
                // deemed safely spendable, except where soft itself was
                // floored above the caps on a tiny clock (soft is the floor).
                if (soft <= (t * 6) / 10) {
                    EXPECT_LE(hard, std::max(soft, (t * 6) / 10)) << "t=" << t;
                    EXPECT_LE(hard, std::max(soft, t - reserve)) << "t=" << t;
                }
            }
        }
    }
}

TEST(UciHardTimeBudget, NeverCommitsMoreThanHalfTheClockToOneMove) {
    // The scramble guard. Below time <= 3.33*inc the 3x allowance would
    // otherwise reach cap60 and spend 60% of a near-flagged clock on a single
    // move. Assert the halving clamp across the whole low-clock band, and that
    // it stays inert on a normal TC.
    for (long long t = 20; t <= 4000; t += 20) {
        for (long long inc : {0LL, 50LL, 100LL, 500LL}) {
            const long long soft = compute_time_budget_ms(t, inc, 0);
            const long long hard = compute_hard_time_budget_ms(t, inc, 0);
            // Half-clock guard, except where soft itself is already above it
            // (soft is a floor the extension may never undercut).
            EXPECT_LE(hard, std::max(soft, t / 2)) << "t=" << t << " inc=" << inc;
            EXPECT_GE(hard, soft) << "t=" << t << " inc=" << inc;
        }
    }
    // Inert during normal play: 10+0.1 is bounded by 3x soft (1650), not t/2 (5000).
    EXPECT_EQ(compute_hard_time_budget_ms(10000, 100, 0), 1650);
}

TEST(UciHardTimeBudget, UnknownClockGetsNoExtension) {
    // The negative-clock branch is a guess, not a measured slice — tripling it
    // would invent time the GUI never told us we had.
    EXPECT_EQ(compute_hard_time_budget_ms(-1, 2000, 0), compute_time_budget_ms(-1, 2000, 0));
    EXPECT_EQ(compute_hard_time_budget_ms(-1, 0, 0), compute_time_budget_ms(-1, 0, 0));
}

// (parse_go_command wiring for both bounds is covered in test_uci_go_parsing.cpp,
//  which owns the token-splitting helper.)

// --- Strict numeric parsing (shared setoption/go parser) ----------------------

TEST(UciTimeAllocation, ParseSpinClampedRejectsJunkAndClamps) {
    long long v = -777;
    EXPECT_FALSE(parse_spin_clamped("", 0, 100, v));
    EXPECT_FALSE(parse_spin_clamped("nope", 0, 100, v));
    EXPECT_FALSE(parse_spin_clamped("12junk", 0, 100, v)) << "stoi prefix-parse must be gone";
    EXPECT_FALSE(parse_spin_clamped("-", 0, 100, v));
    EXPECT_FALSE(parse_spin_clamped("1 2", 0, 100, v));
    EXPECT_EQ(v, -777) << "failed parse must not touch out";

    EXPECT_TRUE(parse_spin_clamped("42", 0, 100, v));
    EXPECT_EQ(v, 42);
    EXPECT_TRUE(parse_spin_clamped("+7", 0, 100, v));
    EXPECT_EQ(v, 7);
    EXPECT_TRUE(parse_spin_clamped("-50", 0, 100, v));
    EXPECT_EQ(v, 0) << "negative clamps to lo (flagging GUIs send negative clocks)";
    EXPECT_TRUE(parse_spin_clamped("999999999999999999999", 0, 2000000000LL, v));
    EXPECT_EQ(v, 1000000000LL) << "overflow saturates at 1e9, then clamps";
}

// --- handle_go end-to-end: hostile input stays alive, tiny clocks move fast ----

TEST(UciTimeAllocation, TinyClockGoReturnsPromptly) {
    Huginn::init();
    UCIInterface uci;
    uci.handle_position({"position", "startpos"});

    std::ostringstream oss;
    auto old_buf = std::cout.rdbuf(oss.rdbuf());
    auto t0 = std::chrono::steady_clock::now();
    uci.handle_go({"go", "wtime", "10", "btime", "10"});
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count();
    std::cout.rdbuf(old_buf);

    EXPECT_LT(elapsed, 1000)
        << "10ms clock took " << elapsed << "ms (old code budgeted 50ms it did not have)";
    EXPECT_NE(oss.str().find("bestmove"), std::string::npos);
}

TEST(UciTimeAllocation, JunkGoTokensDoNotCrashOrHang) {
    Huginn::init();
    UCIInterface uci;
    uci.handle_position({"position", "startpos"});

    std::ostringstream oss;
    auto old_buf = std::cout.rdbuf(oss.rdbuf());
    // Junk wtime is ignored; the valid movetime bounds the search.
    uci.handle_go({"go", "wtime", "12junk", "movetime", "100", "movestogo", "nope"});
    std::cout.rdbuf(old_buf);

    EXPECT_NE(oss.str().find("bestmove"), std::string::npos);
}
