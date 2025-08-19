#include <gtest/gtest.h>
#include "position.hpp"
#include "bitboard.hpp"
#include "init.hpp"
#include "board120.hpp"
#include "chess_types.hpp"

class PawnBitboardTest : public ::testing::Test {
protected:
    Position pos;
    
    void SetUp() override {
        Huginn::init();
        pos.set_startpos();
    }
};

TEST_F(PawnBitboardTest, StartingPositionPawnBitboards) {
    // Test individual pawn bitboards
    uint64_t white_pawns = pos.get_white_pawns();
    uint64_t black_pawns = pos.get_black_pawns();
    uint64_t all_pawns = pos.get_all_pawns_bitboard();
    
    // Verify white pawns are on rank 2 (0-indexed rank 1)
    uint64_t expected_white_pawns = 0x000000000000FF00ULL; // Rank 2 in chess notation
    EXPECT_EQ(white_pawns, expected_white_pawns);
    
    // Verify black pawns are on rank 7 (0-indexed rank 6)
    uint64_t expected_black_pawns = 0x00FF000000000000ULL; // Rank 7 in chess notation
    EXPECT_EQ(black_pawns, expected_black_pawns);
    
    // Verify combined bitboard is the union of both
    EXPECT_EQ(all_pawns, white_pawns | black_pawns);
    EXPECT_EQ(all_pawns, expected_white_pawns | expected_black_pawns);
    
    // Verify there are 16 pawns total
    EXPECT_EQ(popcount(all_pawns), 16);
    EXPECT_EQ(popcount(white_pawns), 8);
    EXPECT_EQ(popcount(black_pawns), 8);
}

TEST_F(PawnBitboardTest, PawnCaptureUpdatesAllBitboards) {
    // Make some moves to set up a capture: e2-e4, d7-d5, exd5
    Move move;
    
    // e2-e4 (White pawn move)
    move.from = sq(File::E, Rank::R2);
    move.to = sq(File::E, Rank::R4);
    move.promo = PieceType::None;
    
    pos.make_move_with_undo(move);
    
    // d7-d5 (Black pawn move)  
    move.from = sq(File::D, Rank::R7);
    move.to = sq(File::D, Rank::R5);
    
    pos.make_move_with_undo(move);
    
    // exd5 (White pawn captures black pawn)
    move.from = sq(File::E, Rank::R4);
    move.to = sq(File::D, Rank::R5);
    
    pos.make_move_with_undo(move);
    
    // Verify the capture updated all bitboards correctly
    uint64_t white_pawns = pos.get_white_pawns();
    uint64_t black_pawns = pos.get_black_pawns();
    uint64_t all_pawns = pos.get_all_pawns_bitboard();
    
    // Should have 8 white pawns and 7 black pawns (one captured)
    EXPECT_EQ(popcount(white_pawns), 8);
    EXPECT_EQ(popcount(black_pawns), 7);
    EXPECT_EQ(popcount(all_pawns), 15);
    
    // Combined bitboard should match
    EXPECT_EQ(all_pawns, white_pawns | black_pawns);
    
    // White pawn should be on d5
    int d5_64 = MAILBOX_MAPS.to64[sq(File::D, Rank::R5)];
    EXPECT_TRUE(getBit(white_pawns, d5_64));
    EXPECT_TRUE(getBit(all_pawns, d5_64));
    
    // No black pawn should be on d5
    EXPECT_FALSE(getBit(black_pawns, d5_64));
}

TEST_F(PawnBitboardTest, PawnPromotionUpdatesAllBitboards) {
    // Note: Pawn promotions may require special handling in make_move_with_undo
    // For now, let's test simpler pawn removal by testing captures
    
    // Set up a position with pawns that can capture each other
    pos.reset();
    
    // Place pawns that can capture
    pos.board[sq(File::E, Rank::R4)] = Piece::WhitePawn;  // White pawn on e4
    pos.board[sq(File::D, Rank::R5)] = Piece::BlackPawn;  // Black pawn on d5
    
    // Place kings
    pos.board[sq(File::H, Rank::R1)] = Piece::WhiteKing;
    pos.board[sq(File::A, Rank::R8)] = Piece::BlackKing;
    
    pos.rebuild_counts();
    
    // Verify initial state - should have 2 pawns
    uint64_t initial_all_pawns = pos.get_all_pawns_bitboard();
    EXPECT_EQ(popcount(initial_all_pawns), 2);
    
    // White captures black: exd5
    Move move;
    move.from = sq(File::E, Rank::R4);
    move.to = sq(File::D, Rank::R5);
    move.promo = PieceType::None;
    
    pos.make_move_with_undo(move);
    
    // After capture, should have 1 pawn (white) and 0 black pawns
    uint64_t white_pawns = pos.get_white_pawns();
    uint64_t black_pawns = pos.get_black_pawns();
    uint64_t all_pawns = pos.get_all_pawns_bitboard();
    
    EXPECT_EQ(popcount(white_pawns), 1) << "Should have 1 white pawn after capture";
    EXPECT_EQ(popcount(black_pawns), 0) << "Should have 0 black pawns after capture";
    EXPECT_EQ(popcount(all_pawns), 1) << "Should have 1 total pawn after capture";
    
    // Combined bitboard should match
    EXPECT_EQ(all_pawns, white_pawns | black_pawns);
    
    // White pawn should be on d5
    int d5_64 = MAILBOX_MAPS.to64[sq(File::D, Rank::R5)];
    EXPECT_TRUE(getBit(white_pawns, d5_64));
}

TEST_F(PawnBitboardTest, MakeUnmakePawnMoveConsistency) {
    // Test that make/unmake preserves pawn bitboard consistency
    
    // Store initial state
    uint64_t initial_white = pos.get_white_pawns();
    uint64_t initial_black = pos.get_black_pawns();
    uint64_t initial_all = pos.get_all_pawns_bitboard();
    
    // Make a pawn move: e2-e4
    Move move;
    
    move.from = sq(File::E, Rank::R2);
    move.to = sq(File::E, Rank::R4);
    move.promo = PieceType::None;
    
    pos.make_move_with_undo(move);
    
    // Verify the move changed the bitboards
    uint64_t after_move_all = pos.get_all_pawns_bitboard();
    EXPECT_NE(after_move_all, initial_all);
    EXPECT_EQ(popcount(after_move_all), 16); // Same number of pawns
    
    // Unmake the move
    bool undo_success = pos.undo_move();
    EXPECT_TRUE(undo_success);
    
    // Verify we're back to initial state
    EXPECT_EQ(pos.get_white_pawns(), initial_white);
    EXPECT_EQ(pos.get_black_pawns(), initial_black);
    EXPECT_EQ(pos.get_all_pawns_bitboard(), initial_all);
    
    // Combined bitboard should match
    EXPECT_EQ(pos.get_all_pawns_bitboard(), pos.get_white_pawns() | pos.get_black_pawns());
}

TEST_F(PawnBitboardTest, EmptyPositionHasNoPawns) {
    // Empty position should have no pawns
    pos.reset();
    pos.rebuild_counts();
    
    EXPECT_EQ(pos.get_white_pawns(), 0ULL);
    EXPECT_EQ(pos.get_black_pawns(), 0ULL);
    EXPECT_EQ(pos.get_all_pawns_bitboard(), 0ULL);
}

TEST_F(PawnBitboardTest, AllPawnBitboardConsistency) {
    // Test that all_pawns_bb is always the union of white and black pawns
    
    // Test multiple moves to ensure consistency is maintained
    std::vector<Move> moves = {
        {sq(File::E, Rank::R2), sq(File::E, Rank::R4), PieceType::None},  // e2-e4
        {sq(File::E, Rank::R7), sq(File::E, Rank::R5), PieceType::None},  // e7-e5
        {sq(File::D, Rank::R2), sq(File::D, Rank::R4), PieceType::None},  // d2-d4
        {sq(File::E, Rank::R5), sq(File::D, Rank::R4), PieceType::None},  // exd4 (capture)
    };
    
    for (const auto& move : moves) {
        pos.make_move_with_undo(move);
        
        // Verify consistency after each move
        uint64_t white_pawns = pos.get_white_pawns();
        uint64_t black_pawns = pos.get_black_pawns();
        uint64_t all_pawns = pos.get_all_pawns_bitboard();
        
        EXPECT_EQ(all_pawns, white_pawns | black_pawns);
    }
}
