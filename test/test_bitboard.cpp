// test_bitboard.cpp
#include <gtest/gtest.h>
#include "bitboard.hpp"
#include <sstream>
#include <iostream>

class BitboardTest : public ::testing::Test {
protected:
    void SetUp() override {
        bb = EMPTY_BB;
    }
    
    Bitboard bb;
};

// Test basic bit manipulation macros
TEST_F(BitboardTest, BitManipulationMacros) {
    // Test setBit/addBit
    setBit(bb, 0);  // a1
    EXPECT_TRUE(getBit(bb, 0));
    EXPECT_EQ(bb, 1ULL);
    
    addBit(bb, 7);  // h1
    EXPECT_TRUE(getBit(bb, 7));
    EXPECT_EQ(bb, 129ULL);  // bits 0 and 7 set
    
    // Test popBit/PopBit
    popBit(bb, 0);
    EXPECT_FALSE(getBit(bb, 0));
    EXPECT_TRUE(getBit(bb, 7));
    EXPECT_EQ(bb, 128ULL);
    
    PopBit(bb, 7);
    EXPECT_FALSE(getBit(bb, 7));
    EXPECT_EQ(bb, EMPTY_BB);
}

// Test square indexing functions
TEST_F(BitboardTest, SquareIndexing) {
    // Test square_from_file_rank
    EXPECT_EQ(square_from_file_rank(0, 0), 0);   // a1
    EXPECT_EQ(square_from_file_rank(7, 0), 7);   // h1
    EXPECT_EQ(square_from_file_rank(0, 7), 56);  // a8
    EXPECT_EQ(square_from_file_rank(7, 7), 63);  // h8
    
    // Test file_of_square and rank_of_square
    EXPECT_EQ(file_of_square(0), 0);   // a1 -> file A (0)
    EXPECT_EQ(rank_of_square(0), 0);   // a1 -> rank 1 (0)
    EXPECT_EQ(file_of_square(63), 7);  // h8 -> file H (7)
    EXPECT_EQ(rank_of_square(63), 7);  // h8 -> rank 8 (7)
    EXPECT_EQ(file_of_square(28), 4);  // e4 -> file E (4)
    EXPECT_EQ(rank_of_square(28), 3);  // e4 -> rank 4 (3)
}

// Test square conversion between 64 and 120 indexing
TEST_F(BitboardTest, SquareConversion) {
    // Test some known conversions
    EXPECT_EQ(sq64_to_sq120(0), 21);   // a1: 0 -> 21
    EXPECT_EQ(sq64_to_sq120(7), 28);   // h1: 7 -> 28
    EXPECT_EQ(sq64_to_sq120(56), 91);  // a8: 56 -> 91
    EXPECT_EQ(sq64_to_sq120(63), 98);  // h8: 63 -> 98
    EXPECT_EQ(sq64_to_sq120(28), 55);  // e4: 28 -> 55
    
    // Test reverse conversion
    EXPECT_EQ(sq120_to_sq64(21), 0);   // a1: 21 -> 0
    EXPECT_EQ(sq120_to_sq64(28), 7);   // h1: 28 -> 7
    EXPECT_EQ(sq120_to_sq64(91), 56);  // a8: 91 -> 56
    EXPECT_EQ(sq120_to_sq64(98), 63);  // h8: 98 -> 63
    EXPECT_EQ(sq120_to_sq64(55), 28);  // e4: 55 -> 28
    
    // Test invalid squares
    EXPECT_EQ(sq64_to_sq120(-1), -1);
    EXPECT_EQ(sq64_to_sq120(64), -1);
    EXPECT_EQ(sq120_to_sq64(20), -1);  // Off-board
    EXPECT_EQ(sq120_to_sq64(29), -1);  // Off-board
}

// Test round-trip conversion
TEST_F(BitboardTest, RoundTripConversion) {
    for (int sq64 = 0; sq64 < 64; ++sq64) {
        int sq120 = sq64_to_sq120(sq64);
        int back_to_64 = sq120_to_sq64(sq120);
        EXPECT_EQ(back_to_64, sq64) << "Failed round-trip for square " << sq64;
    }
}

// Test utility functions
TEST_F(BitboardTest, UtilityFunctions) {
    // Test empty bitboard
    EXPECT_TRUE(is_empty(EMPTY_BB));
    EXPECT_FALSE(is_empty(1ULL));
    
    // Test popcount
    EXPECT_EQ(popcount(EMPTY_BB), 0);
    EXPECT_EQ(popcount(1ULL), 1);
    EXPECT_EQ(popcount(3ULL), 2);  // bits 0 and 1
    EXPECT_EQ(popcount(FULL_BB), 64);
    
    // Test countBit (should be identical to popcount)
    EXPECT_EQ(countBit(EMPTY_BB), 0);
    EXPECT_EQ(countBit(1ULL), 1);
    EXPECT_EQ(countBit(3ULL), 2);  // bits 0 and 1
    EXPECT_EQ(countBit(FULL_BB), 64);
    EXPECT_EQ(countBit(0xAAAAAAAAAAAAAAAAULL), 32);  // alternating bits pattern
    
    // Test is_set
    setBit(bb, 20);
    EXPECT_TRUE(is_set(bb, 20));
    EXPECT_FALSE(is_set(bb, 19));
    EXPECT_FALSE(is_set(bb, 21));
}

// Test LSB functions
TEST_F(BitboardTest, LSBFunctions) {
    // Test get_lsb
    EXPECT_EQ(get_lsb(EMPTY_BB), -1);
    EXPECT_EQ(get_lsb(1ULL), 0);
    EXPECT_EQ(get_lsb(2ULL), 1);
    EXPECT_EQ(get_lsb(4ULL), 2);
    EXPECT_EQ(get_lsb(8ULL), 3);
    EXPECT_EQ(get_lsb(6ULL), 1);  // bits 1 and 2, LSB is 1
    
    // Test pop_lsb
    bb = 6ULL;  // bits 1 and 2 set
    EXPECT_EQ(pop_lsb(bb), 1);
    EXPECT_EQ(bb, 4ULL);  // only bit 2 remaining
    EXPECT_EQ(pop_lsb(bb), 2);
    EXPECT_EQ(bb, EMPTY_BB);
    EXPECT_EQ(pop_lsb(bb), -1);  // empty bitboard
}

// Test file and rank constants
TEST_F(BitboardTest, FileAndRankConstants) {
    // Test file constants
    EXPECT_EQ(popcount(FILE_A), 8);
    EXPECT_EQ(popcount(FILE_H), 8);
    EXPECT_TRUE(is_set(FILE_A, 0));   // a1
    EXPECT_TRUE(is_set(FILE_A, 56));  // a8
    EXPECT_TRUE(is_set(FILE_H, 7));   // h1
    EXPECT_TRUE(is_set(FILE_H, 63));  // h8
    
    // Test rank constants
    EXPECT_EQ(popcount(RANK_1), 8);
    EXPECT_EQ(popcount(RANK_8), 8);
    EXPECT_TRUE(is_set(RANK_1, 0));   // a1
    EXPECT_TRUE(is_set(RANK_1, 7));   // h1
    EXPECT_TRUE(is_set(RANK_8, 56));  // a8
    EXPECT_TRUE(is_set(RANK_8, 63));  // h8
    
    // Test array access
    EXPECT_EQ(FILE_BB[0], FILE_A);
    EXPECT_EQ(FILE_BB[7], FILE_H);
    EXPECT_EQ(RANK_BB[0], RANK_1);
    EXPECT_EQ(RANK_BB[7], RANK_8);
}

// Test printBitboard function (basic functionality)
TEST_F(BitboardTest, PrintBitboardBasic) {
    // This test mainly checks that the function doesn't crash
    // Visual verification would be done manually
    
    // Redirect cout to capture output
    std::streambuf* orig = std::cout.rdbuf();
    std::ostringstream oss;
    std::cout.rdbuf(oss.rdbuf());
    
    // Test empty bitboard
    printBitboard(EMPTY_BB);
    std::string output1 = oss.str();
    EXPECT_FALSE(output1.empty());
    EXPECT_TRUE(output1.find("a b c d e f g h") != std::string::npos);
    
    // Clear stream
    oss.str("");
    oss.clear();
    
    // Test bitboard with some pieces
    Bitboard test_bb = 0;
    setBit(test_bb, 0);  // a1
    setBit(test_bb, 7);  // h1
    setBit(test_bb, 56); // a8
    setBit(test_bb, 63); // h8
    
    printBitboard(test_bb, 'X', '.');
    std::string output2 = oss.str();
    EXPECT_FALSE(output2.empty());
    EXPECT_TRUE(output2.find("X") != std::string::npos);
    EXPECT_TRUE(output2.find(".") != std::string::npos);
    
    // Restore cout
    std::cout.rdbuf(orig);
}

// Test corner squares specifically
TEST_F(BitboardTest, CornerSquares) {
    Bitboard corners = 0;
    setBit(corners, 0);   // a1
    setBit(corners, 7);   // h1
    setBit(corners, 56);  // a8
    setBit(corners, 63);  // h8
    
    EXPECT_EQ(popcount(corners), 4);
    EXPECT_TRUE(is_set(corners, 0));
    EXPECT_TRUE(is_set(corners, 7));
    EXPECT_TRUE(is_set(corners, 56));
    EXPECT_TRUE(is_set(corners, 63));
    
    // Test that middle squares are not set
    EXPECT_FALSE(is_set(corners, 28));  // e4
    EXPECT_FALSE(is_set(corners, 35));  // d5
}
