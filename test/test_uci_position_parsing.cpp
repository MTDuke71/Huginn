#include <gtest/gtest.h>
#include "uci.hpp"
#include <sstream>

// BACKLOG #60 parser-purity refactor: these used to only assert
// EXPECT_NO_THROW(uci.handle_position(tokens)), which passes even if
// handle_position silently ignores the command entirely (exactly what
// happened with the malformed-castling-flag LCT2 EPD entry that motivated
// this refactor — see docs/SEARCH_AND_EVAL.md). Every case now asserts the
// EXACT resulting FEN via current_position(), the same accessor the #54
// transactionality tests use (test_audit_criticals.cpp).

class UCIPositionTest : public ::testing::Test {
protected:
    UCIInterface uci;  // constructor sets the root to the standard startpos

    std::vector<std::string> split_command(const std::string& command) {
        std::vector<std::string> tokens;
        std::istringstream iss(command);
        std::string token;
        while (iss >> token) tokens.push_back(token);
        return tokens;
    }
};

namespace {
constexpr const char* kStartposFen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}

TEST_F(UCIPositionTest, ParseStartpos) {
    uci.handle_position(split_command("position startpos"));
    EXPECT_EQ(uci.current_position().to_fen(), kStartposFen);
}

TEST_F(UCIPositionTest, ParseStartposWithMoves) {
    // 1.e4 e5 -- both pawn double-pushes leave no capturable EP square (#59
    // normalizes it away), halfmove resets on each pawn move, fullmove -> 2.
    uci.handle_position(split_command("position startpos moves e2e4 e7e5"));
    EXPECT_EQ(uci.current_position().to_fen(),
              "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
}

TEST_F(UCIPositionTest, ParseFEN) {
    uci.handle_position(split_command(std::string("position fen ") + kStartposFen));
    EXPECT_EQ(uci.current_position().to_fen(), kStartposFen);
}

TEST_F(UCIPositionTest, ParseFENWithMoves) {
    // The base FEN's "e3" EP square isn't actually capturable by Black here
    // (no black pawn has moved yet), so #59 normalizes it away on load; after
    // ...e5 the position converges to the exact same FEN as
    // ParseStartposWithMoves -- two different construction paths, one
    // position.
    uci.handle_position(split_command(
        "position fen rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1 moves e7e5"));
    EXPECT_EQ(uci.current_position().to_fen(),
              "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
}

TEST_F(UCIPositionTest, ParseFENMissingCountersIsRejectedNotDefaulted) {
    // set_from_fen requires exactly 6 space-separated fields; a FEN missing
    // halfmove/fullmove (4 fields here) is REJECTED outright, not completed
    // with defaults -- the previous test's EXPECT_NO_THROW made this look
    // like tolerated "variable length" input when it's actually a straight
    // rejection. The root must stay exactly as it was (the ctor's startpos).
    uci.handle_position(split_command(
        "position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"));
    EXPECT_EQ(uci.current_position().to_fen(), kStartposFen);
}

TEST_F(UCIPositionTest, ParseInvalidPositionTypeIsRejected) {
    // Neither "startpos" nor "fen" -- rejected, root left exactly as it was.
    uci.handle_position(split_command("position invalid"));
    EXPECT_EQ(uci.current_position().to_fen(), kStartposFen);
}

TEST_F(UCIPositionTest, ParseMalformedCastlingFlagIsRejected) {
    // The exact bug class found in lct2.epd's LCTII.CMB.06: a castling flag
    // asserting a right whose rook isn't actually on its home square.
    // validate_uci_position must reject it (BACKLOG #54 transactionality) --
    // the caller must NOT silently end up analysing some other position.
    uci.handle_position(split_command(
        "position fen 2r1k2r/2pn1pp1/1p3n1p/p3PP2/4q2B/P1P5/2Q1N1PP/R4RK1 w q - 0 1"));
    EXPECT_EQ(uci.current_position().to_fen(), kStartposFen);
}
