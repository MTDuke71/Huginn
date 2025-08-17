// test_init.cpp
#include <gtest/gtest.h>
#include "init.hpp"
#include "zobrist.hpp"

class InitTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Each test gets a fresh initialization
        Huginn::init();
    }
};

TEST_F(InitTest, EngineInitialization) {
    // Verify that init() marks the engine as initialized
    EXPECT_TRUE(Huginn::is_initialized());
    
    // Verify that Zobrist tables are initialized
    EXPECT_TRUE(Zobrist::Initialized);
}

TEST_F(InitTest, ZobristTablesPopulated) {
    // Verify that Zobrist tables contain non-zero values
    // (extremely unlikely to be zero with a good RNG)
    bool has_nonzero_piece = false;
    for (int p = 0; p < 12 && !has_nonzero_piece; ++p) {
        for (int s = 0; s < 120; ++s) {  // Now checking all 120 squares
            if (Zobrist::Piece[p][s] != 0) {
                has_nonzero_piece = true;
                break;
            }
        }
    }
    EXPECT_TRUE(has_nonzero_piece) << "Zobrist piece tables should contain non-zero values";
    
    // Check that Side is non-zero
    EXPECT_NE(Zobrist::Side, 0ULL) << "Zobrist Side should be non-zero";
    
    // Check that at least some Castle values are non-zero
    bool has_nonzero_castle = false;
    for (int i = 0; i < 16; ++i) {
        if (Zobrist::Castle[i] != 0) {
            has_nonzero_castle = true;
            break;
        }
    }
    EXPECT_TRUE(has_nonzero_castle) << "Zobrist Castle tables should contain non-zero values";
}

TEST_F(InitTest, MultipleInitCallsSafe) {
    // Verify that calling init() multiple times is safe
    Huginn::init();
    EXPECT_TRUE(Huginn::is_initialized());
    
    // Store some values before second init
    U64 original_side = Zobrist::Side;
    U64 original_piece = Zobrist::Piece[0][0];
    
    // Call init again
    Huginn::init();
    EXPECT_TRUE(Huginn::is_initialized());
    
    // Values should be unchanged (not re-randomized)
    EXPECT_EQ(Zobrist::Side, original_side);
    EXPECT_EQ(Zobrist::Piece[0][0], original_piece);
}
