#include <gtest/gtest.h>
#include "parallel_movegen.hpp"
#include "movegen_enhanced.hpp"
#include "position.hpp"
#include <algorithm>

class ParallelMoveGenTest : public ::testing::Test {
protected:
    Position pos;
    S_MOVELIST sequential_pseudo, parallel_pseudo;
    S_MOVELIST sequential_legal, parallel_legal;
    
    void SetUp() override {
        pos.reset();
        sequential_pseudo.clear();
        parallel_pseudo.clear();
        sequential_legal.clear();
        parallel_legal.clear();
        
        // Configure parallel move generation for testing
        ParallelConfig::auto_configure();
    }
    
    void compare_move_lists(const S_MOVELIST& expected, const S_MOVELIST& actual, const std::string& context) {
        EXPECT_EQ(expected.count, actual.count) << "Move count mismatch in " << context;
        
        if (expected.count == actual.count) {
            // Sort both lists for comparison (moves might be generated in different order)
            std::vector<S_MOVE> expected_moves(expected.moves, expected.moves + expected.count);
            std::vector<S_MOVE> actual_moves(actual.moves, actual.moves + actual.count);
            
            auto move_comparator = [](const S_MOVE& a, const S_MOVE& b) {
                if (a.get_from() != b.get_from()) return a.get_from() < b.get_from();
                if (a.get_to() != b.get_to()) return a.get_to() < b.get_to();
                return a.move < b.move;  // Compare full move encoding for flags
            };
            
            std::sort(expected_moves.begin(), expected_moves.end(), move_comparator);
            std::sort(actual_moves.begin(), actual_moves.end(), move_comparator);
            
            for (size_t i = 0; i < expected_moves.size(); ++i) {
                EXPECT_EQ(expected_moves[i].get_from(), actual_moves[i].get_from()) 
                    << "Move " << i << " from square mismatch in " << context;
                EXPECT_EQ(expected_moves[i].get_to(), actual_moves[i].get_to()) 
                    << "Move " << i << " to square mismatch in " << context;
                EXPECT_EQ(expected_moves[i].move, actual_moves[i].move) 
                    << "Move " << i << " encoding mismatch in " << context;
            }
        }
    }
};

TEST_F(ParallelMoveGenTest, StartingPositionPseudoLegal) {
    // Test starting position
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    generate_all_moves(pos, sequential_pseudo);
    ParallelMoveGenerator::generate_all_moves_parallel(pos, parallel_pseudo);
    
    compare_move_lists(sequential_pseudo, parallel_pseudo, "starting position pseudo-legal");
}

TEST_F(ParallelMoveGenTest, StartingPositionLegal) {
    // Test starting position legal moves
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    generate_legal_moves_enhanced(pos, sequential_legal);
    ParallelMoveGenerator::generate_legal_moves_parallel(pos, parallel_legal);
    
    compare_move_lists(sequential_legal, parallel_legal, "starting position legal");
}

TEST_F(ParallelMoveGenTest, KiwipetePseudoLegal) {
    // Test Kiwipete position (complex tactical position)
    pos.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    
    generate_all_moves(pos, sequential_pseudo);
    ParallelMoveGenerator::generate_all_moves_parallel(pos, parallel_pseudo);
    
    compare_move_lists(sequential_pseudo, parallel_pseudo, "Kiwipete pseudo-legal");
}

TEST_F(ParallelMoveGenTest, KiwipeteLegal) {
    // Test Kiwipete position legal moves
    pos.set_from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    
    generate_legal_moves_enhanced(pos, sequential_legal);
    ParallelMoveGenerator::generate_legal_moves_parallel(pos, parallel_legal);
    
    compare_move_lists(sequential_legal, parallel_legal, "Kiwipete legal");
}

TEST_F(ParallelMoveGenTest, ComplexMiddlegamePseudoLegal) {
    // Test complex middlegame position
    pos.set_from_fen("r1bq1rk1/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQ - 0 1");
    
    generate_all_moves(pos, sequential_pseudo);
    ParallelMoveGenerator::generate_all_moves_parallel(pos, parallel_pseudo);
    
    compare_move_lists(sequential_pseudo, parallel_pseudo, "complex middlegame pseudo-legal");
}

TEST_F(ParallelMoveGenTest, ComplexMiddlegameLegal) {
    // Test complex middlegame position legal moves
    pos.set_from_fen("r1bq1rk1/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQ - 0 1");
    
    generate_legal_moves_enhanced(pos, sequential_legal);
    ParallelMoveGenerator::generate_legal_moves_parallel(pos, parallel_legal);
    
    compare_move_lists(sequential_legal, parallel_legal, "complex middlegame legal");
}

TEST_F(ParallelMoveGenTest, EndgamePseudoLegal) {
    // Test endgame position
    pos.set_from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    
    generate_all_moves(pos, sequential_pseudo);
    ParallelMoveGenerator::generate_all_moves_parallel(pos, parallel_pseudo);
    
    compare_move_lists(sequential_pseudo, parallel_pseudo, "endgame pseudo-legal");
}

TEST_F(ParallelMoveGenTest, EndgameLegal) {
    // Test endgame position legal moves
    pos.set_from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    
    generate_legal_moves_enhanced(pos, sequential_legal);
    ParallelMoveGenerator::generate_legal_moves_parallel(pos, parallel_legal);
    
    compare_move_lists(sequential_legal, parallel_legal, "endgame legal");
}

TEST_F(ParallelMoveGenTest, TacticalPositionPseudoLegal) {
    // Test tactical position with many pieces
    pos.set_from_fen("r2qkb1r/pp2nppp/3p4/2pNN1B1/2BnP3/3P4/PPP2PPP/R2QK2R w KQkq - 0 1");
    
    generate_all_moves(pos, sequential_pseudo);
    ParallelMoveGenerator::generate_all_moves_parallel(pos, parallel_pseudo);
    
    compare_move_lists(sequential_pseudo, parallel_pseudo, "tactical position pseudo-legal");
}

TEST_F(ParallelMoveGenTest, TacticalPositionLegal) {
    // Test tactical position legal moves
    pos.set_from_fen("r2qkb1r/pp2nppp/3p4/2pNN1B1/2BnP3/3P4/PPP2PPP/R2QK2R w KQkq - 0 1");
    
    generate_legal_moves_enhanced(pos, sequential_legal);
    ParallelMoveGenerator::generate_legal_moves_parallel(pos, parallel_legal);
    
    compare_move_lists(sequential_legal, parallel_legal, "tactical position legal");
}

TEST_F(ParallelMoveGenTest, ConfigurationTest) {
    // Test that parallel configuration is working
    ParallelConfig::auto_configure();
    
    EXPECT_GE(ParallelConfig::thread_count, 1);
    EXPECT_LE(ParallelConfig::thread_count, ParallelConfig::MAX_THREADS);
    
    std::cout << "Parallel configuration:" << std::endl;
    std::cout << "  Hardware threads: " << std::thread::hardware_concurrency() << std::endl;
    std::cout << "  Configured threads: " << ParallelConfig::thread_count << std::endl;
    std::cout << "  Parallel generation: " << (ParallelConfig::use_parallel_generation ? "ON" : "OFF") << std::endl;
    std::cout << "  Parallel legal: " << (ParallelConfig::use_parallel_legal ? "ON" : "OFF") << std::endl;
}

TEST_F(ParallelMoveGenTest, SequentialFallbackTest) {
    // Test that parallel methods fall back to sequential when appropriate
    
    // Force disable parallel processing
    bool orig_parallel_gen = ParallelConfig::use_parallel_generation;
    bool orig_parallel_legal = ParallelConfig::use_parallel_legal;
    int orig_thread_count = ParallelConfig::thread_count;
    
    ParallelConfig::use_parallel_generation = false;
    ParallelConfig::use_parallel_legal = false;
    ParallelConfig::thread_count = 1;
    
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    generate_all_moves(pos, sequential_pseudo);
    ParallelMoveGenerator::generate_all_moves_parallel(pos, parallel_pseudo);
    
    generate_legal_moves_enhanced(pos, sequential_legal);
    ParallelMoveGenerator::generate_legal_moves_parallel(pos, parallel_legal);
    
    compare_move_lists(sequential_pseudo, parallel_pseudo, "fallback pseudo-legal");
    compare_move_lists(sequential_legal, parallel_legal, "fallback legal");
    
    // Restore original settings
    ParallelConfig::use_parallel_generation = orig_parallel_gen;
    ParallelConfig::use_parallel_legal = orig_parallel_legal;
    ParallelConfig::thread_count = orig_thread_count;
}
