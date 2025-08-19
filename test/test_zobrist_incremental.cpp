#include <gtest/gtest.h>
#include "../src/position.hpp"
#include "../src/zobrist.hpp"
#include "../src/chess_types.hpp"
#include "../src/init.hpp"

class ZobristIncrementalTest : public ::testing::Test {
protected:
    Position pos;
    
    void SetUp() override {
        Huginn::init(); // Initialize Zobrist tables
        pos.set_startpos();
    }
};

TEST_F(ZobristIncrementalTest, IncrementalXORMatchesFullComputation) {
    // Get initial Zobrist key using full computation
    uint64_t full_key_before = Zobrist::compute(pos);
    EXPECT_EQ(pos.zobrist_key, full_key_before) << "Position zobrist_key should match computed key";
    
    // Make a simple pawn move: e2-e4
    S_MOVE move = make_move(sq(File::E, Rank::R2), sq(File::E, Rank::R4));
    
    // Make the move (this uses incremental XOR updates)
    pos.make_move_with_undo(move);
    
    // Compute the full Zobrist key and compare with incremental result
    uint64_t full_key_after = Zobrist::compute(pos);
    EXPECT_EQ(pos.zobrist_key, full_key_after) 
        << "Incremental XOR update should match full computation after move";
    
    // Undo the move
    bool undo_success = pos.undo_move();
    EXPECT_TRUE(undo_success) << "Undo move should succeed";
    
    // Verify the Zobrist key is restored to original value
    uint64_t full_key_restored = Zobrist::compute(pos);
    EXPECT_EQ(pos.zobrist_key, full_key_restored) 
        << "Zobrist key should be correctly restored after undo";
    EXPECT_EQ(pos.zobrist_key, full_key_before) 
        << "Zobrist key should match original value after undo";
}

TEST_F(ZobristIncrementalTest, CaptureMovesUpdateCorrectly) {
    // Set up a position with a capture possible
    // Place a black pawn on e4 that can be captured
    pos.set(sq(File::E, Rank::R4), make_piece(Color::Black, PieceType::Pawn));
    pos.rebuild_counts(); // Recalculate after manual piece placement
    
    // Get the Zobrist key using full computation
    uint64_t full_key_before = Zobrist::compute(pos);
    pos.zobrist_key = full_key_before; // Sync the position's key
    
    // Make a capture move: d2xe4
    S_MOVE move = make_capture(sq(File::D, Rank::R2), sq(File::E, Rank::R4), PieceType::Pawn);
    
    // Make the move (this uses incremental XOR updates)
    pos.make_move_with_undo(move);
    
    // Verify incremental update matches full computation
    uint64_t full_key_after = Zobrist::compute(pos);
    EXPECT_EQ(pos.zobrist_key, full_key_after) 
        << "Incremental XOR update should handle captures correctly";
    
    // Undo the move
    bool undo_success = pos.undo_move();
    EXPECT_TRUE(undo_success) << "Undo move should succeed";
    
    // Verify restoration
    uint64_t full_key_restored = Zobrist::compute(pos);
    EXPECT_EQ(pos.zobrist_key, full_key_restored) 
        << "Zobrist key should be correctly restored after capture undo";
}

TEST_F(ZobristIncrementalTest, MultipleMovesInSequence) {
    uint64_t original_key = Zobrist::compute(pos);
    pos.zobrist_key = original_key;
    
    // Make several moves in sequence
    std::vector<S_MOVE> moves = {
        make_move(sq(File::E, Rank::R2), sq(File::E, Rank::R4)),  // e2-e4
        make_move(sq(File::D, Rank::R2), sq(File::D, Rank::R3)),  // d2-d3
        make_move(sq(File::F, Rank::R1), sq(File::E, Rank::R2))   // Bf1-e2
    };
    
    for (const auto& move : moves) {
        pos.make_move_with_undo(move);
        
        // Verify each move maintains Zobrist correctness
        uint64_t full_key = Zobrist::compute(pos);
        EXPECT_EQ(pos.zobrist_key, full_key) 
            << "Incremental update should match full computation at each step";
    }
    
    // Undo all moves in reverse order
    for (int i = moves.size() - 1; i >= 0; --i) {
        bool undo_success = pos.undo_move();
        EXPECT_TRUE(undo_success) << "Undo move should succeed for move " << i;
        
        uint64_t full_key = Zobrist::compute(pos);
        EXPECT_EQ(pos.zobrist_key, full_key) 
            << "Zobrist key should be correct after undo " << i;
    }
    
    // Should be back to original position
    EXPECT_EQ(pos.zobrist_key, original_key) 
        << "Should return to original Zobrist key after all undos";
}

TEST_F(ZobristIncrementalTest, XORPropertyWorks) {
    // The fundamental property of Zobrist hashing: A ⊕ B ⊕ B = A
    // Making and unmaking the same move should return to original key
    
    uint64_t original_key = Zobrist::compute(pos);
    pos.zobrist_key = original_key;
    
    S_MOVE move = make_move(sq(File::E, Rank::R2), sq(File::E, Rank::R4));
    
    // Apply the same move 1000 times (even number)
    for (int i = 0; i < 1000; ++i) {
        pos.make_move_with_undo(move);
        pos.undo_move();
    }
    
    // Should be back to original key due to XOR property
    EXPECT_EQ(pos.zobrist_key, original_key) 
        << "XOR property should ensure return to original key after even number of operations";
}
