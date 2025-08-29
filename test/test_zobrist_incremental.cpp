#include <gtest/gtest.h>
#include "position.hpp"
#include "zobrist.hpp"
#include "chess_types.hpp"
#include "init.hpp"

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
    
    // Make the move using make_move_with_undo (this uses incremental XOR updates)
    pos.make_move_with_undo(move);
    
    // Compute the full Zobrist key and compare with incremental result
    uint64_t full_key_after = Zobrist::compute(pos);
    EXPECT_EQ(pos.zobrist_key, full_key_after) 
        << "Incremental XOR update should match full computation after move";
    
    // Undo the move using undo_move
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
    // Set up a simple position with a safe capture
    // Remove the d2 pawn and place a white pawn on d4 instead
    pos.clear_piece(sq(File::D, Rank::R2));  // Remove original d2 pawn
    pos.add_piece(sq(File::D, Rank::R4), make_piece(Color::White, PieceType::Pawn));  // Add pawn on d4
    
    // Place a black pawn on e5 that can be captured safely
    pos.set(sq(File::E, Rank::R5), make_piece(Color::Black, PieceType::Pawn));
    pos.rebuild_counts(); // Recalculate after manual piece placement
    pos.update_zobrist_key(); // IMPORTANT: Recalculate Zobrist hash after manual changes
    
    // Get the Zobrist key using full computation
    uint64_t full_key_before = Zobrist::compute(pos);
    pos.zobrist_key = full_key_before; // Sync the position's key
    
    // Make a safe capture move: d4xe5 (this won't expose the king)
    S_MOVE move = make_capture(sq(File::D, Rank::R4), sq(File::E, Rank::R5), PieceType::Pawn);
    
    // Make the move using VICE MakeMove function (this uses incremental XOR updates)
    int move_result = pos.MakeMove(move);
    EXPECT_EQ(move_result, 1) << "Move should be legal";
    
    // Verify incremental update matches full computation
    uint64_t full_key_after = Zobrist::compute(pos);
    EXPECT_EQ(pos.zobrist_key, full_key_after) 
        << "Incremental XOR update should handle captures correctly";
    
    // Undo the move using VICE TakeMove function
    pos.TakeMove();
    
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
        int move_result = pos.MakeMove(move);
        EXPECT_EQ(move_result, 1) << "Move should be legal";
        
        // Verify each move maintains Zobrist correctness
        uint64_t full_key = Zobrist::compute(pos);
        EXPECT_EQ(pos.zobrist_key, full_key) 
            << "Incremental update should match full computation at each step";
    }
    
    // Undo all moves in reverse order using VICE TakeMove function
    for (int i = moves.size() - 1; i >= 0; --i) {
        pos.TakeMove();
        
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
