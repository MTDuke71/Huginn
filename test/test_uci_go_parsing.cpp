#include <gtest/gtest.h>
#include "uci.hpp"
#include <sstream>

class UCIGoParsingTest : public ::testing::Test {
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

// VICE Part 69: Test depth parsing
TEST_F(UCIGoParsingTest, ParseDepth) {
    auto tokens = split_command("go depth 6");
    EXPECT_NO_THROW(uci.handle_go(tokens));
}

// VICE Part 69: Test movetime parsing
TEST_F(UCIGoParsingTest, ParseMovetime) {
    auto tokens = split_command("go movetime 5000");
    EXPECT_NO_THROW(uci.handle_go(tokens));
}

// VICE Part 69: Test wtime/btime parsing
TEST_F(UCIGoParsingTest, ParseWtimeBtime) {
    auto tokens = split_command("go wtime 120000 btime 115000");
    EXPECT_NO_THROW(uci.handle_go(tokens));
}

// VICE Part 69: Test winc/binc parsing
TEST_F(UCIGoParsingTest, ParseWincBinc) {
    auto tokens = split_command("go wtime 60000 btime 60000 winc 1000 binc 1000");
    EXPECT_NO_THROW(uci.handle_go(tokens));
}

// VICE Part 69: Test movestogo parsing
TEST_F(UCIGoParsingTest, ParseMovestogo) {
    auto tokens = split_command("go wtime 120000 btime 120000 movestogo 40");
    EXPECT_NO_THROW(uci.handle_go(tokens));
}

// VICE Part 69: Test combined time control parameters
TEST_F(UCIGoParsingTest, ParseComplexTimeControl) {
    auto tokens = split_command("go wtime 600000 btime 580000 winc 2000 binc 2000 movestogo 30");
    EXPECT_NO_THROW(uci.handle_go(tokens));
}

// Test error handling for invalid parameters
TEST_F(UCIGoParsingTest, ParseInvalidValues) {
    // Should handle gracefully without crashing
    auto tokens1 = split_command("go depth abc");
    EXPECT_NO_THROW(uci.handle_go(tokens1));
    
    auto tokens2 = split_command("go movetime -100");
    EXPECT_NO_THROW(uci.handle_go(tokens2));
    
    auto tokens3 = split_command("go wtime invalid_number");
    EXPECT_NO_THROW(uci.handle_go(tokens3));
}

// Test unknown parameters
TEST_F(UCIGoParsingTest, ParseUnknownParameters) {
    auto tokens = split_command("go unknown_param 123 depth 5");
    EXPECT_NO_THROW(uci.handle_go(tokens));
}

// Test empty go command
TEST_F(UCIGoParsingTest, ParseEmptyGo) {
    auto tokens = split_command("go");
    EXPECT_NO_THROW(uci.handle_go(tokens));
}
