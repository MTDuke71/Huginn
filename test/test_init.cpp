// test_init.cpp
#include <gtest/gtest.h>
#include "init.hpp"
#include "zobrist.hpp"
#include "attack_tables.hpp"
#include "bitboard.hpp"
#include "chess_types.hpp"

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
        for (int s = 0; s < 64; ++s) {
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

TEST_F(InitTest, AttackTableFixtureSpotChecks) {
    // Replaces the orphan verify_attack_tables() prod function. Same three
    // fixture cases, now expressed with per-assertion diagnostics that
    // actually run on every test invocation.
    // e4 = file 4, rank 3 (0-indexed) = 3*8 + 4 = 28
    // d2 = 11, f6 = 45, d5 = 35, f5 = 37
    EXPECT_NE(knight_attacks[28] & (1ULL << 11), 0u)
        << "knight on e4 should attack d2";
    EXPECT_NE(knight_attacks[28] & (1ULL << 45), 0u)
        << "knight on e4 should attack f6";
    EXPECT_EQ(popcount(king_attacks[28]), 8)
        << "king on e4 should attack 8 adjacent squares";
    EXPECT_NE(pawn_attacks[static_cast<int>(Color::White)][28] & (1ULL << 35), 0u)
        << "white pawn on e4 should attack d5";
    EXPECT_NE(pawn_attacks[static_cast<int>(Color::White)][28] & (1ULL << 37), 0u)
        << "white pawn on e4 should attack f5";
}
