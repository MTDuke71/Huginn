#include <gtest/gtest.h>
#include "uci_utils.hpp"
#include <sstream>

// BACKLOG #60 parser-purity refactor: these used to wrap a real
// UCIInterface::handle_go() call (which starts an actual search) in
// EXPECT_NO_THROW, so a parser that silently produced the wrong limits for
// every input would still pass every test here. handle_go's token-parsing
// logic now lives in the pure, side-effect-free parse_go_command
// (uci_utils.cpp) — no UCIInterface, no search thread, just exact assertions
// on the resulting MinimalLimits.

namespace {
std::vector<std::string> split_command(const std::string& command) {
    std::vector<std::string> tokens;
    std::istringstream iss(command);
    std::string token;
    while (iss >> token) tokens.push_back(token);
    return tokens;
}
} // namespace

TEST(UCIGoParsingTest, ParseDepth) {
    bool infinite_requested = false;
    auto limits = parse_go_command(split_command("go depth 6"), Color::White, infinite_requested);
    EXPECT_EQ(limits.max_depth, 6);
    EXPECT_TRUE(limits.infinite);   // depth-only search ignores time
    EXPECT_EQ(limits.max_time_ms, 0);
    EXPECT_FALSE(infinite_requested);  // no literal "infinite" token
}

TEST(UCIGoParsingTest, ParseMovetime) {
    bool infinite_requested = false;
    auto limits = parse_go_command(split_command("go movetime 5000"), Color::White, infinite_requested);
    EXPECT_EQ(limits.max_time_ms, 5000);
    EXPECT_FALSE(limits.infinite);
    EXPECT_EQ(limits.max_depth, 25);  // default, untouched by movetime
}

TEST(UCIGoParsingTest, ParseWtimeBtime) {
    bool infinite_requested = false;
    // White to move: uses wtime, sudden-death branch (movestogo=0):
    // alloc = 120000/20 + 0 = 6000; reserve/cap60 don't bind at this clock.
    auto limits = parse_go_command(split_command("go wtime 120000 btime 115000"), Color::White, infinite_requested);
    EXPECT_EQ(limits.max_time_ms, 6000);
    EXPECT_FALSE(limits.infinite);

    // Same command, Black to move: uses btime (115000/20 = 5750).
    auto limits_black = parse_go_command(split_command("go wtime 120000 btime 115000"), Color::Black, infinite_requested);
    EXPECT_EQ(limits_black.max_time_ms, 5750);
}

TEST(UCIGoParsingTest, ParseWincBinc) {
    bool infinite_requested = false;
    // White: alloc = 60000/20 + 1000/2 = 3000 + 500 = 3500.
    auto limits = parse_go_command(split_command("go wtime 60000 btime 60000 winc 1000 binc 1000"),
                                    Color::White, infinite_requested);
    EXPECT_EQ(limits.max_time_ms, 3500);
}

TEST(UCIGoParsingTest, ParseMovestogo) {
    bool infinite_requested = false;
    // Classical allocation: alloc = 120000/40 + 0 = 3000.
    auto limits = parse_go_command(split_command("go wtime 120000 btime 120000 movestogo 40"),
                                    Color::White, infinite_requested);
    EXPECT_EQ(limits.max_time_ms, 3000);
}

TEST(UCIGoParsingTest, ParseComplexTimeControl) {
    bool infinite_requested = false;
    // alloc = 600000/30 + 2000/2 = 20000 + 1000 = 21000.
    auto limits = parse_go_command(split_command("go wtime 600000 btime 580000 winc 2000 binc 2000 movestogo 30"),
                                    Color::White, infinite_requested);
    EXPECT_EQ(limits.max_time_ms, 21000);
}

TEST(UCIGoParsingTest, ParseInvalidDepthLeavesItUnspecified) {
    // Non-numeric depth value is rejected outright -> depth stays unset ->
    // falls through to the bare-`go` default (no wtime/btime/movetime either).
    bool infinite_requested = false;
    auto limits = parse_go_command(split_command("go depth abc"), Color::White, infinite_requested);
    EXPECT_EQ(limits.max_depth, 25);
    EXPECT_FALSE(limits.infinite);
    EXPECT_EQ(limits.max_time_ms, 5000);
}

TEST(UCIGoParsingTest, ParseNegativeMovetimeClampsToOneMs) {
    // parse_spin_clamped clamps out-of-range numerics rather than rejecting
    // them; -100 clamps to the lo bound (1ms), not "unset".
    bool infinite_requested = false;
    auto limits = parse_go_command(split_command("go movetime -100"), Color::White, infinite_requested);
    EXPECT_EQ(limits.max_time_ms, 1);
    EXPECT_FALSE(limits.infinite);
}

TEST(UCIGoParsingTest, ParseNonNumericWtimeLeavesClockUnset) {
    // A genuinely non-numeric value token is rejected (not clamped); with no
    // usable wtime/btime/movetime/depth, this is a bare-`go` default.
    bool infinite_requested = false;
    auto limits = parse_go_command(split_command("go wtime invalid_number"), Color::White, infinite_requested);
    EXPECT_EQ(limits.max_time_ms, 5000);
    EXPECT_FALSE(limits.infinite);
    EXPECT_EQ(limits.max_depth, 25);
}

TEST(UCIGoParsingTest, ParseUnknownParametersAreSkippedNotConsumed) {
    // Unknown keywords don't consume a following value token (only
    // recognized keywords do) -- "unknown_param" and its bare "123" are both
    // skipped independently, and "depth 5" still parses correctly after them.
    bool infinite_requested = false;
    auto limits = parse_go_command(split_command("go unknown_param 123 depth 5"), Color::White, infinite_requested);
    EXPECT_EQ(limits.max_depth, 5);
    EXPECT_TRUE(limits.infinite);
}

TEST(UCIGoParsingTest, ParseEmptyGoUsesBareDefault) {
    bool infinite_requested = false;
    auto limits = parse_go_command(split_command("go"), Color::White, infinite_requested);
    EXPECT_EQ(limits.max_depth, 25);
    EXPECT_FALSE(limits.infinite);
    EXPECT_EQ(limits.max_time_ms, 5000);
    EXPECT_FALSE(infinite_requested);
}

TEST(UCIGoParsingTest, ParseInfiniteWithNoDepthUsesMaxDepth) {
    // #56: `go infinite` must run until `stop` -- full MAX_DEPTH range when
    // no explicit depth accompanies it.
    bool infinite_requested = false;
    auto limits = parse_go_command(split_command("go infinite"), Color::White, infinite_requested);
    EXPECT_TRUE(infinite_requested);
    EXPECT_TRUE(limits.infinite);
    EXPECT_EQ(limits.max_depth, Huginn::MAX_DEPTH);
    EXPECT_EQ(limits.max_time_ms, 0);
}

TEST(UCIGoParsingTest, ParseInfiniteWithExplicitDepthKeepsThatDepth) {
    // An explicit depth alongside "infinite" is NOT overridden to MAX_DEPTH.
    bool infinite_requested = false;
    auto limits = parse_go_command(split_command("go infinite depth 10"), Color::White, infinite_requested);
    EXPECT_TRUE(infinite_requested);
    EXPECT_EQ(limits.max_depth, 10);
}
