#include <gtest/gtest.h>
#include <chrono>
#include "movegen_enhanced.hpp"
#include "position.hpp"
#include "init.hpp"

class EnhancedMoveGenTest : public ::testing::Test {
protected:
    void SetUp() override {
        Huginn::init();
    }
    
    Position pos;
    S_MOVELIST moves;
};

TEST_F(EnhancedMoveGenTest, StartingPositionMoveCount) {
    pos.set_startpos();
    generate_all_moves(pos, moves);
    
    EXPECT_EQ(moves.size(), 20);  // 20 moves from starting position
    
    // Check that all moves have score 0 (quiet moves)
    for (int i = 0; i < moves.size(); ++i) {
        EXPECT_EQ(moves[i].score, 0);
    }
}

TEST_F(EnhancedMoveGenTest, LegalMoveFiltering) {
    pos.set_startpos();
    
    S_MOVELIST pseudo_moves;
    S_MOVELIST legal_moves;
    
    generate_all_moves(pos, pseudo_moves);
    generate_legal_moves_enhanced(pos, legal_moves);
    
    // Starting position has no illegal moves
    EXPECT_EQ(pseudo_moves.size(), legal_moves.size());
    EXPECT_EQ(legal_moves.size(), 20);
}

TEST_F(EnhancedMoveGenTest, MoveScoring) {
    // Position with capture opportunities
    pos.set_from_fen("rnbqkb1r/pppp1ppp/5n2/4p3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 2 3");
    
    generate_all_moves(pos, moves);
    moves.sort_by_score();
    
    // First move should be highest scoring (capture)
    bool found_capture = false;
    for (int i = 0; i < moves.size(); ++i) {
        if (moves[i].is_capture()) {
            EXPECT_GT(moves[i].score, 1000000);  // Capture scores start at 1,000,000+
            found_capture = true;
            break;
        }
    }
    EXPECT_TRUE(found_capture);
}

TEST_F(EnhancedMoveGenTest, PromotionScoring) {
    // Position with pawn promotion
    pos.set_from_fen("8/P7/8/8/8/8/8/8 w - - 0 1");
    
    generate_all_moves(pos, moves);
    
    EXPECT_EQ(moves.size(), 4);  // 4 promotion moves (Q, R, B, N)
    
    // All moves should be promotions with high scores
    for (int i = 0; i < moves.size(); ++i) {
        EXPECT_TRUE(moves[i].is_promotion());
        EXPECT_GT(moves[i].score, 2000000);  // Promotion scores start at 2,000,000+
    }
    
    moves.sort_by_score();
    
    // Queen promotion should score highest
    EXPECT_EQ(moves[0].get_promoted(), PieceType::Queen);
}

TEST_F(EnhancedMoveGenTest, CastlingScoring) {
    // Position where castling is available
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    
    generate_all_moves(pos, moves);
    
    bool found_castle = false;
    for (int i = 0; i < moves.size(); ++i) {
        if (moves[i].is_castle()) {
            EXPECT_EQ(moves[i].score, 50000);  // Castle score
            found_castle = true;
        }
    }
    EXPECT_TRUE(found_castle);
}

TEST_F(EnhancedMoveGenTest, EnPassantScoring) {
    // Position with en passant available
    pos.set_from_fen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
    
    generate_all_moves(pos, moves);
    
    bool found_en_passant = false;
    for (int i = 0; i < moves.size(); ++i) {
        if (moves[i].is_en_passant()) {
            EXPECT_EQ(moves[i].score, 1000105);  // En passant score
            found_en_passant = true;
        }
    }
    EXPECT_TRUE(found_en_passant);
}

TEST_F(EnhancedMoveGenTest, MVVLVAScoring) {
    // Position where queen can capture different pieces
    pos.set_from_fen("8/8/3r1n2/8/3Q4/8/8/8 w - - 0 1");
    
    generate_all_moves(pos, moves);
    moves.sort_by_score();
    
    // Find the capture moves
    std::vector<S_MOVE> capture_moves;
    for (int i = 0; i < moves.size(); ++i) {
        if (moves[i].is_capture()) {
            capture_moves.push_back(moves[i]);
        }
    }
    
    EXPECT_EQ(capture_moves.size(), 2);  // Queen captures rook and knight
    
    // Capturing rook (value 500) should score higher than capturing knight (value 320)
    if (capture_moves.size() >= 2) {
        // Both captures are by queen (value 900), so victim value determines order
        int rook_capture_score = 1000000 + (10 * 500) - 900;  // 1004100
        int knight_capture_score = 1000000 + (10 * 320) - 900;  // 1002200
        
        bool found_higher_rook = false;
        for (const auto& move : capture_moves) {
            if (move.get_captured() == PieceType::Rook) {
                EXPECT_EQ(move.score, rook_capture_score);
                found_higher_rook = true;
            } else if (move.get_captured() == PieceType::Knight) {
                EXPECT_EQ(move.score, knight_capture_score);
            }
        }
        EXPECT_TRUE(found_higher_rook);
    }
}

TEST_F(EnhancedMoveGenTest, IteratorInterface) {
    pos.set_startpos();
    generate_all_moves(pos, moves);
    
    // Test range-based for loop
    int count = 0;
    for (const auto& move : moves) {
        EXPECT_EQ(move.score, 0);  // All starting moves are quiet
        count++;
    }
    EXPECT_EQ(count, 20);
    
    // Test iterator methods
    EXPECT_EQ(moves.end() - moves.begin(), 20);
}

TEST_F(EnhancedMoveGenTest, ArrayAccessInterface) {
    pos.set_startpos();
    generate_all_moves(pos, moves);
    
    EXPECT_EQ(moves.size(), 20);
    
    // Test array access
    for (int i = 0; i < moves.size(); ++i) {
        EXPECT_EQ(moves[i].score, 0);
    }
}

TEST_F(EnhancedMoveGenTest, PinDetection) {
    // Position where a piece is pinned
    pos.set_from_fen("8/8/8/3k4/8/3r4/3K4/8 w - - 0 1");
    
    S_MOVELIST pseudo_moves;
    S_MOVELIST legal_moves;
    
    generate_all_moves(pos, pseudo_moves);
    generate_legal_moves_enhanced(pos, legal_moves);
    
    // King should have fewer legal moves due to attack by rook
    EXPECT_LT(legal_moves.size(), pseudo_moves.size());
}

TEST_F(EnhancedMoveGenTest, PerformanceCheck) {
    pos.set_startpos();
    
    const int iterations = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        S_MOVELIST test_moves;
        generate_all_moves(pos, test_moves);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should be very fast - less than 1 microsecond per generation on modern hardware
    double avg_time = static_cast<double>(duration.count()) / iterations;
    EXPECT_LT(avg_time, 5.0);  // Less than 5 microseconds per generation
}
