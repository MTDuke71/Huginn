#include <gtest/gtest.h>
#include "../src/position.hpp"
#include "../src/chess_types.hpp"

class IncrementalUpdateTest : public ::testing::Test {
protected:
    Position pos;
    
    void SetUp() override {
        pos.set_startpos();
    }
};

TEST_F(IncrementalUpdateTest, MakeUnmakeMaintainsState) {
    // Save initial state
    auto initial_king_sq = pos.king_sq;
    auto initial_pawns_bb = pos.pawns_bb;
    auto initial_piece_counts = pos.piece_counts;
    
    // Make a simple pawn move: e2-e4
    Move move;
    move.from = sq(File::E, Rank::R2);  // e2
    move.to = sq(File::E, Rank::R4);    // e4
    move.promo = PieceType::None;
    
    // Make the move
    pos.make_move_with_undo(move);
    
    // Verify the pawn bitboard was updated correctly
    // White pawn should have moved from e2 to e4
    int e2_sq64 = MAILBOX_MAPS.to64[sq(File::E, Rank::R2)];
    int e4_sq64 = MAILBOX_MAPS.to64[sq(File::E, Rank::R4)];
    
    EXPECT_FALSE(getBit(pos.pawns_bb[size_t(Color::White)], e2_sq64)) 
        << "White pawn should no longer be on e2";
    EXPECT_TRUE(getBit(pos.pawns_bb[size_t(Color::White)], e4_sq64)) 
        << "White pawn should now be on e4";
    
    // Piece counts should remain the same
    EXPECT_EQ(pos.piece_counts, initial_piece_counts) << "Piece counts should be unchanged";
    
    // King positions should be unchanged
    EXPECT_EQ(pos.king_sq, initial_king_sq) << "King positions should be unchanged";
    
    // Undo the move
    bool undo_success = pos.undo_move();
    EXPECT_TRUE(undo_success) << "Undo move should succeed";
    
    // Verify state is completely restored
    EXPECT_EQ(pos.king_sq, initial_king_sq) << "King positions should be restored";
    EXPECT_EQ(pos.pawns_bb, initial_pawns_bb) << "Pawn bitboards should be restored";
    EXPECT_EQ(pos.piece_counts, initial_piece_counts) << "Piece counts should be restored";
    
    // Verify the actual board position
    EXPECT_EQ(pos.at(move.from), make_piece(Color::White, PieceType::Pawn)) 
        << "White pawn should be back on e2";
    EXPECT_EQ(pos.at(move.to), Piece::None) 
        << "e4 should be empty again";
}

TEST_F(IncrementalUpdateTest, CaptureMoveMaintainsCorrectCounts) {
    // Set up a position with a capture possible
    // Place a black pawn on e4 that can be captured
    pos.set(sq(File::E, Rank::R4), make_piece(Color::Black, PieceType::Pawn));
    pos.rebuild_counts(); // Recalculate after manual piece placement
    
    // Save initial state
    auto initial_piece_counts = pos.piece_counts;
    auto initial_pawns_bb = pos.pawns_bb;
    
    // Make a capture move: d2xd4 (assuming we move the d2 pawn to capture on e4)
    Move move;
    move.from = sq(File::D, Rank::R2);  // d2
    move.to = sq(File::E, Rank::R4);    // e4 (capture black pawn)
    move.promo = PieceType::None;
    
    // Make the move
    pos.make_move_with_undo(move);
    
    // Verify piece count decreased by 1 (captured pawn)
    EXPECT_EQ(pos.piece_counts[size_t(PieceType::Pawn)], initial_piece_counts[size_t(PieceType::Pawn)] - 1)
        << "Pawn count should decrease by 1 due to capture";
    
    // Verify black pawn bitboard no longer has the captured pawn
    int e4_sq64 = MAILBOX_MAPS.to64[sq(File::E, Rank::R4)];
    EXPECT_TRUE(getBit(pos.pawns_bb[size_t(Color::White)], e4_sq64)) 
        << "White pawn should be on e4";
    
    // Undo the move
    bool undo_success = pos.undo_move();
    EXPECT_TRUE(undo_success) << "Undo move should succeed";
    
    // Verify state is completely restored
    EXPECT_EQ(pos.piece_counts, initial_piece_counts) << "Piece counts should be restored";
    EXPECT_EQ(pos.pawns_bb, initial_pawns_bb) << "Pawn bitboards should be restored";
    
    // Verify the captured piece is back
    EXPECT_EQ(pos.at(sq(File::E, Rank::R4)), make_piece(Color::Black, PieceType::Pawn)) 
        << "Black pawn should be restored on e4";
    EXPECT_EQ(pos.at(sq(File::D, Rank::R2)), make_piece(Color::White, PieceType::Pawn)) 
        << "White pawn should be back on d2";
}

TEST_F(IncrementalUpdateTest, KingMoveMaintainsKingSquare) {
    // Clear some pieces to make room for king move
    pos.set(sq(File::E, Rank::R2), Piece::None); // Remove e2 pawn
    pos.rebuild_counts();
    
    // Save initial state
    auto initial_king_sq = pos.king_sq;
    
    // Make a king move: Ke1-Ke2
    Move move;
    move.from = sq(File::E, Rank::R1);  // e1
    move.to = sq(File::E, Rank::R2);    // e2
    move.promo = PieceType::None;
    
    // Make the move
    pos.make_move_with_undo(move);
    
    // Verify white king square was updated
    EXPECT_EQ(pos.king_sq[size_t(Color::White)], sq(File::E, Rank::R2)) 
        << "White king square should be updated to e2";
    EXPECT_EQ(pos.king_sq[size_t(Color::Black)], initial_king_sq[size_t(Color::Black)]) 
        << "Black king square should be unchanged";
    
    // Undo the move
    bool undo_success = pos.undo_move();
    EXPECT_TRUE(undo_success) << "Undo move should succeed";
    
    // Verify king square is restored
    EXPECT_EQ(pos.king_sq, initial_king_sq) << "King squares should be restored";
}
