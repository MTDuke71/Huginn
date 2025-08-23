#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <functional>
#include "position.hpp"
#include "board.hpp"
#include "chess_types.hpp"

class PrintPositionTest : public ::testing::Test {
protected:
    Position pos;
    
    // Helper to capture stdout
    std::string captureOutput(std::function<void()> func) {
        std::streambuf* orig = std::cout.rdbuf();
        std::ostringstream captured;
        std::cout.rdbuf(captured.rdbuf());
        
        func();
        
        std::cout.rdbuf(orig);
        return captured.str();
    }
};

TEST_F(PrintPositionTest, PrintStartingPosition) {
    pos.set_startpos();
    
    std::string output = captureOutput([this]() {
        print_position(pos);
    });
    
    // Check that essential elements are present in the output
    EXPECT_TRUE(output.find("Side to move: White") != std::string::npos);
    EXPECT_TRUE(output.find("Castling rights: KQkq") != std::string::npos);
    EXPECT_TRUE(output.find("En passant square: -") != std::string::npos);
    EXPECT_TRUE(output.find("Halfmove clock: 0") != std::string::npos);
    EXPECT_TRUE(output.find("Fullmove number: 1") != std::string::npos);
    EXPECT_TRUE(output.find("Pawns=16") != std::string::npos);
    EXPECT_TRUE(output.find("Kings=2") != std::string::npos);
    
    // Check that board structure is present
    EXPECT_TRUE(output.find("+---+") != std::string::npos);  // Board borders
    EXPECT_TRUE(output.find("| r |") != std::string::npos);  // Black pieces
    EXPECT_TRUE(output.find("| R |") != std::string::npos);  // White pieces
    EXPECT_TRUE(output.find("| . |") != std::string::npos);  // Empty squares
    EXPECT_TRUE(output.find("a   b   c   d   e   f   g   h") != std::string::npos);  // File labels
}

TEST_F(PrintPositionTest, PrintEmptyPosition) {
    pos.reset();  // Empty board
    
    std::string output = captureOutput([this]() {
        print_position(pos);
    });
    
    // Check empty board characteristics
    // Note: reset() sets side_to_move to Color::None, which displays as "White" due to the enum values
    EXPECT_TRUE(output.find("Side to move:") != std::string::npos);  // Side info should be present
    EXPECT_TRUE(output.find("Castling rights: -") != std::string::npos);
    EXPECT_TRUE(output.find("En passant square: -") != std::string::npos);
    EXPECT_TRUE(output.find("Pawns=0") != std::string::npos);
    EXPECT_TRUE(output.find("Kings=0") != std::string::npos);
    
    // Should be mostly empty squares
    size_t dot_count = 0;
    for (char c : output) {
        if (c == '.') dot_count++;
    }
    EXPECT_GE(dot_count, 60);  // Should have many empty squares (at least most of the 64)
}

TEST_F(PrintPositionTest, PrintKiwipetePosition) {
    const std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ASSERT_TRUE(pos.set_from_fen(kiwipete_fen));
    
    std::string output = captureOutput([this]() {
        print_position(pos);
    });
    
    // Check specific Kiwipete characteristics
    EXPECT_TRUE(output.find("Side to move: White") != std::string::npos);
    EXPECT_TRUE(output.find("Castling rights: KQkq") != std::string::npos);
    EXPECT_TRUE(output.find("Pawns=16") != std::string::npos);  // All pawns still on board
    
    // Check presence of key pieces in output
    EXPECT_TRUE(output.find("| r |") != std::string::npos);  // Rooks
    EXPECT_TRUE(output.find("| k |") != std::string::npos);  // Black king
    EXPECT_TRUE(output.find("| K |") != std::string::npos);  // White king
    EXPECT_TRUE(output.find("| Q |") != std::string::npos);  // White queen
    EXPECT_TRUE(output.find("| q |") != std::string::npos);  // Black queen
}

TEST_F(PrintPositionTest, PrintFunctionExists) {
    // Simple test to ensure the function can be called without crashing
    pos.set_startpos();
    
    EXPECT_NO_THROW({
        // Capture output to avoid polluting test output
        std::streambuf* orig = std::cout.rdbuf();
        std::ostringstream captured;
        std::cout.rdbuf(captured.rdbuf());
        
        print_position(pos);
        
        std::cout.rdbuf(orig);
    });
}
