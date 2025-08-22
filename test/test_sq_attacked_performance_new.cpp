#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include <iostream>
#include <iomanip>
#include "../src/position.hpp"
#include "../src/attack_detection.hpp"
#include "../src/board120.hpp"
#include "../src/chess_types.hpp"
#include "../src/squares120.hpp"

class SqAttackedPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize random seed for reproducible tests
        rng.seed(42);
    }
    
    std::mt19937 rng;
    
    Position create_starting_position() {
        Position pos;
        pos.set_startpos();
        return pos;
    }
    
    Position create_middle_game_position() {
        Position pos;
        pos.set_from_fen("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 0 4");
        return pos;
    }
    
    Position create_endgame_position() {
        Position pos;
        pos.set_from_fen("8/2k5/8/4K3/8/8/8/R7 w - - 0 1");
        return pos;
    }
    
    // Benchmark function that tests a position against multiple squares
    double benchmark_position(const Position& pos, Color attacking_color, const std::string& test_name) {
        const int NUM_ITERATIONS = 10000;
        const int NUM_SQUARES = 64; // Test all playable squares
        
        auto start = std::chrono::high_resolution_clock::now();
        
        volatile bool result = false; // Prevent optimization
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {
            for (int r = 0; r < 8; ++r) {
                for (int f = 0; f < 8; ++f) {
                    int test_sq = sq(static_cast<File>(f), static_cast<Rank>(r));
                    result = SqAttacked(test_sq, pos, attacking_color);
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        
        double avg_ns_per_call = double(duration) / (NUM_ITERATIONS * NUM_SQUARES);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << test_name << ": " << avg_ns_per_call << " ns/call" << std::endl;
        
        return avg_ns_per_call;
    }
};

TEST_F(SqAttackedPerformanceTest, StartingPositionPerformance) {
    Position pos = create_starting_position();
    
    std::cout << "\n=== Starting Position Performance ===" << std::endl;
    double white_time = benchmark_position(pos, Color::White, "White attacks");
    double black_time = benchmark_position(pos, Color::Black, "Black attacks");
    
    std::cout << "Average: " << (white_time + black_time) / 2.0 << " ns/call" << std::endl;
    
    // Performance expectation: should be under 1000ns per call for optimized version
    EXPECT_LT((white_time + black_time) / 2.0, 1000.0) << "Performance regression detected";
}

TEST_F(SqAttackedPerformanceTest, MiddleGamePerformance) {
    Position pos = create_middle_game_position();
    
    std::cout << "\n=== Middle Game Position Performance ===" << std::endl;
    double white_time = benchmark_position(pos, Color::White, "White attacks");
    double black_time = benchmark_position(pos, Color::Black, "Black attacks");
    
    std::cout << "Average: " << (white_time + black_time) / 2.0 << " ns/call" << std::endl;
    
    EXPECT_LT((white_time + black_time) / 2.0, 1000.0) << "Performance regression detected";
}

TEST_F(SqAttackedPerformanceTest, EndgamePerformance) {
    Position pos = create_endgame_position();
    
    std::cout << "\n=== Endgame Position Performance ===" << std::endl;
    double white_time = benchmark_position(pos, Color::White, "White attacks");
    double black_time = benchmark_position(pos, Color::Black, "Black attacks");
    
    std::cout << "Average: " << (white_time + black_time) / 2.0 << " ns/call" << std::endl;
    
    // Endgame should be significantly faster due to fewer pieces
    EXPECT_LT((white_time + black_time) / 2.0, 500.0) << "Endgame should be very fast";
}

// Test with various positions to ensure correctness is maintained
TEST_F(SqAttackedPerformanceTest, CorrectnessTest) {
    Position pos = create_starting_position();
    
    // Test some known attack patterns
    EXPECT_TRUE(SqAttacked(sq(File::C, Rank::Three), pos, Color::White));  // b1 knight attacks c3
    EXPECT_TRUE(SqAttacked(sq(File::F, Rank::Three), pos, Color::White));  // g1 knight attacks f3
    EXPECT_FALSE(SqAttacked(sq(File::E, Rank::Four), pos, Color::White)); // Center square not attacked initially
    EXPECT_FALSE(SqAttacked(sq(File::E, Rank::Five), pos, Color::Black)); // Center square not attacked initially
    
    // Test pawn attacks
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    EXPECT_TRUE(SqAttacked(sq(File::D, Rank::Five), pos, Color::White));  // e4 pawn attacks d5
    EXPECT_TRUE(SqAttacked(sq(File::F, Rank::Five), pos, Color::White));  // e4 pawn attacks f5
    EXPECT_FALSE(SqAttacked(sq(File::E, Rank::Five), pos, Color::White)); // Pawn doesn't attack forward
}

// Stress test with random positions
TEST_F(SqAttackedPerformanceTest, RandomPositionStressTest) {
    const int NUM_RANDOM_TESTS = 100;
    double total_time = 0.0;
    
    std::cout << "\n=== Random Position Stress Test ===" << std::endl;
    
    for (int i = 0; i < NUM_RANDOM_TESTS; ++i) {
        // Create a random-ish position by moving pieces from starting position
        Position pos = create_starting_position();
        
        // Test a few squares per position
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int r = 0; r < 8; r += 2) { // Test every other rank
            for (int f = 0; f < 8; f += 2) { // Test every other file
                int test_sq = sq(static_cast<File>(f), static_cast<Rank>(r));
                volatile bool result1 = SqAttacked(test_sq, pos, Color::White);
                volatile bool result2 = SqAttacked(test_sq, pos, Color::Black);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        total_time += duration;
    }
    
    double avg_time = total_time / (NUM_RANDOM_TESTS * 32); // 32 tests per position
    std::cout << "Random positions average: " << avg_time << " ns/call" << std::endl;
    
    EXPECT_LT(avg_time, 1000.0) << "Random position performance regression";
}

// Specific worst-case scenario test
TEST_F(SqAttackedPerformanceTest, WorstCaseScenario) {
    // Position with maximum pieces that could potentially attack
    Position pos;
    pos.set_from_fen("r1bqkb1r/1ppppp1p/p1n2np1/8/8/P1N2NP1/1PPPPP1P/R1BQKB1R w KQkq - 0 1");
    
    std::cout << "\n=== Worst Case Scenario Performance ===" << std::endl;
    double white_time = benchmark_position(pos, Color::White, "White attacks (worst case)");
    double black_time = benchmark_position(pos, Color::Black, "Black attacks (worst case)");
    
    std::cout << "Worst case average: " << (white_time + black_time) / 2.0 << " ns/call" << std::endl;
    
    // Even worst case should be reasonable
    EXPECT_LT((white_time + black_time) / 2.0, 2000.0) << "Worst case performance too slow";
}
