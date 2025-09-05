#include <gtest/gtest.h>
#include "uci.hpp"
#include <sstream>

class UCIPositionTest : public ::testing::Test {
protected:
    UCIInterface uci;
    
    std::vector<std::string> split_command(const std::string& command) {
        std::vector<std::string> tokens;
        std::istringstream iss(command);
        std::string token;
        while (iss >> token) tokens.push_back(token);
        return tokens;
    }
};

TEST_F(UCIPositionTest, ParseStartpos) {
    // VICE Part 68: "position startpos"
    auto tokens = split_command("position startpos");
    EXPECT_NO_THROW(uci.handle_position(tokens));
}

TEST_F(UCIPositionTest, ParseStartposWithMoves) {
    // VICE Part 68: "position startpos moves e2e4 e7e5"
    auto tokens = split_command("position startpos moves e2e4 e7e5");
    EXPECT_NO_THROW(uci.handle_position(tokens));
}

TEST_F(UCIPositionTest, ParseFEN) {
    // VICE Part 68: "position fen [FEN_string]"
    auto tokens = split_command("position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    EXPECT_NO_THROW(uci.handle_position(tokens));
}

TEST_F(UCIPositionTest, ParseFENWithMoves) {
    // VICE Part 68: FEN followed by moves
    auto tokens = split_command("position fen rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1 moves e7e5");
    EXPECT_NO_THROW(uci.handle_position(tokens));
}

TEST_F(UCIPositionTest, ParseVariableLengthFEN) {
    // Test robustness: FEN with different component counts
    auto tokens = split_command("position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");
    EXPECT_NO_THROW(uci.handle_position(tokens));
}

TEST_F(UCIPositionTest, ParseInvalidPosition) {
    // Should handle gracefully
    auto tokens = split_command("position invalid");
    EXPECT_NO_THROW(uci.handle_position(tokens));
}
