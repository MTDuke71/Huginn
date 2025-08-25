#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "king_optimizations.hpp"
#include "move.hpp"

class KingOptimizationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up common test positions
    }
    
    // Helper function to compare two move lists
    static bool compare_move_lists(const S_MOVELIST& list1, const S_MOVELIST& list2) {
        if (list1.count != list2.count) return false;
        
        std::vector<uint32_t> moves1, moves2;
        for (int i = 0; i < list1.count; ++i) {
            moves1.push_back(list1.moves[i].move);
        }
        for (int i = 0; i < list2.count; ++i) {
            moves2.push_back(list2.moves[i].move);
        }
        
        std::sort(moves1.begin(), moves1.end());
        std::sort(moves2.begin(), moves2.end());
        
        return moves1 == moves2;
    }
    
    // Test king moves for equivalence between original and optimized
    void test_king_move_equivalence(const std::string& fen, const std::string& position_name) {
        Position pos;
        pos.set_from_fen(fen);
        
        // Generate moves using original function
        S_MOVELIST original_list;
        generate_king_moves(pos, original_list, pos.side_to_move);
        
        // Generate moves using optimized function
        S_MOVELIST optimized_list;
        KingOptimizations::generate_king_moves_optimized(pos, optimized_list, pos.side_to_move);
        
        // They should be identical
        EXPECT_TRUE(compare_move_lists(original_list, optimized_list)) 
            << "King move lists don't match for position: " << position_name
            << "\nFEN: " << fen
            << "\nOriginal count: " << original_list.count
            << "\nOptimized count: " << optimized_list.count;
    }
};

// Test 1: Starting position king moves
TEST_F(KingOptimizationTest, StartingPositionKingMoves) {
    test_king_move_equivalence(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "Starting Position"
    );
}

// Test 2: Kiwipete position
TEST_F(KingOptimizationTest, KiwipetePosition) {
    test_king_move_equivalence(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "Kiwipete"
    );
}

// Test 3: King can castle kingside
TEST_F(KingOptimizationTest, KingsideCastling) {
    test_king_move_equivalence(
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        "Kingside Castling Available"
    );
}

// Test 4: King can castle queenside only
TEST_F(KingOptimizationTest, QueensideCastlingOnly) {
    test_king_move_equivalence(
        "r3k2r/8/8/8/8/8/8/R3K2R w Qq - 0 1",
        "Queenside Castling Only"
    );
}

// Test 5: No castling rights
TEST_F(KingOptimizationTest, NoCastlingRights) {
    test_king_move_equivalence(
        "r3k2r/8/8/8/8/8/8/R3K2R w - - 0 1",
        "No Castling Rights"
    );
}

// Test 6: King in center of board
TEST_F(KingOptimizationTest, KingInCenter) {
    test_king_move_equivalence(
        "8/8/8/3K4/8/8/8/k7 w - - 0 1",
        "King In Center"
    );
}

// Test 7: King blocked by own pieces
TEST_F(KingOptimizationTest, KingBlockedByOwnPieces) {
    test_king_move_equivalence(
        "8/8/8/2PPP3/2PKP3/2PPP3/8/k7 w - - 0 1",
        "King Blocked By Own Pieces"
    );
}

// Test 8: King with capture opportunities
TEST_F(KingOptimizationTest, KingWithCaptures) {
    test_king_move_equivalence(
        "8/8/8/2ppp3/2pKp3/2ppp3/8/k7 w - - 0 1",
        "King With Captures"
    );
}

// Test 9: Castling blocked by pieces
TEST_F(KingOptimizationTest, CastlingBlockedByPieces) {
    test_king_move_equivalence(
        "r1b1k1nr/8/8/8/8/8/8/R1B1K1NR w KQkq - 0 1",
        "Castling Blocked By Pieces"
    );
}

// Test 10: Castling blocked by attack
TEST_F(KingOptimizationTest, CastlingBlockedByAttack) {
    test_king_move_equivalence(
        "r3k2r/8/8/4r3/8/8/8/R3K2R w KQkq - 0 1",
        "Castling Blocked By Attack"
    );
}

// Test 11: Black king moves
TEST_F(KingOptimizationTest, BlackKingMoves) {
    test_king_move_equivalence(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 1",
        "Black King Moves"
    );
}

// Test 12: Endgame position with king activity
TEST_F(KingOptimizationTest, EndgameKingActivity) {
    test_king_move_equivalence(
        "8/2k5/8/8/8/8/2K5/8 w - - 0 1",
        "Endgame King Activity"
    );
}

// Performance test comparing original vs optimized
TEST_F(KingOptimizationTest, PerformanceComparison) {
    std::vector<std::string> test_positions = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        "8/8/8/3K4/8/8/8/k7 w - - 0 1",
        "8/2k5/8/8/8/8/2K5/8 w - - 0 1"
    };
    
    const int iterations = 10000;
    
    auto start_original = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST list;
            generate_king_moves(pos, list, pos.side_to_move);
        }
    }
    auto end_original = std::chrono::high_resolution_clock::now();
    
    auto start_optimized = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST list;
            KingOptimizations::generate_king_moves_optimized(pos, list, pos.side_to_move);
        }
    }
    auto end_optimized = std::chrono::high_resolution_clock::now();
    
    auto original_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_original - start_original);
    auto optimized_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_optimized - start_optimized);
    
    std::cout << "Original king generation: " << original_duration.count() << " μs" << std::endl;
    std::cout << "Optimized king generation: " << optimized_duration.count() << " μs" << std::endl;
    
    if (optimized_duration.count() > 0) {
        double improvement = (double(original_duration.count()) / double(optimized_duration.count()) - 1.0) * 100.0;
        std::cout << "Performance improvement: " << std::fixed << std::setprecision(1) << improvement << "%" << std::endl;
    }
}

// Test individual optimization components
TEST_F(KingOptimizationTest, CastlingViabilityCheck) {
    Position pos;
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    
    // Both sides should be able to castle both ways
    EXPECT_TRUE(KingOptimizations::can_castle_theoretically(pos, Color::White, true));
    EXPECT_TRUE(KingOptimizations::can_castle_theoretically(pos, Color::White, false));
    EXPECT_TRUE(KingOptimizations::can_castle_theoretically(pos, Color::Black, true));
    EXPECT_TRUE(KingOptimizations::can_castle_theoretically(pos, Color::Black, false));
    
    // Remove castling rights
    pos.set_from_fen("r3k2r/8/8/8/8/8/8/R3K2R w - - 0 1");
    EXPECT_FALSE(KingOptimizations::can_castle_theoretically(pos, Color::White, true));
    EXPECT_FALSE(KingOptimizations::can_castle_theoretically(pos, Color::White, false));
    EXPECT_FALSE(KingOptimizations::can_castle_theoretically(pos, Color::Black, true));
    EXPECT_FALSE(KingOptimizations::can_castle_theoretically(pos, Color::Black, false));
}

TEST_F(KingOptimizationTest, OrdinaryKingMovesOnly) {
    Position pos;
    pos.set_from_fen("8/8/8/3K4/8/8/8/k7 w - - 0 1");  // King in center, no castling
    
    S_MOVELIST ordinary_list;
    KingOptimizations::generate_ordinary_king_moves(pos, ordinary_list, Color::White);
    
    S_MOVELIST full_list;
    generate_king_moves(pos, full_list, Color::White);
    
    // Should be identical since no castling is possible
    EXPECT_TRUE(compare_move_lists(ordinary_list, full_list));
    EXPECT_EQ(8, ordinary_list.count);  // King in center has 8 moves
}
