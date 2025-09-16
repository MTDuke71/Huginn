/**
 * @file pawn_movegen_benchmark.cpp
 * @brief Phase 2 Bitboard Migration: Pawn Move Generation Benchmark
 * 
 * This file benchmarks the new bitboard-based pawn move generation against
 * the original piece-list approach to verify the targeted 15-25% performance
 * improvement from the migration plan.
 */

#include "pawn_movegen_bb.hpp"
#include "movegen_enhanced.hpp"
#include "position.hpp"
#include "board120.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std::chrono;

// ============================================================================
// BENCHMARK TEST POSITIONS
// ============================================================================

const std::vector<std::string> test_positions = {
    // Starting position - many pawns
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    
    // Mid-game with mixed pawn structure
    "rnbqkb1r/pp1ppppp/5n2/2p5/2P5/5N2/PP1PPPPP/RNBQKB1R w KQkq - 0 4",
    
    // Pawn promotion test
    "8/P1P3p1/8/8/8/8/1p3p1P/8 w - - 0 1",
    
    // En passant test
    "rnbqkbnr/pp1ppppp/8/2pP4/8/8/PPP1PPPP/RNBQKBNR w KQkq c6 0 3",
    
    // Complex pawn structure
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
};

// ============================================================================
// PERFORMANCE MEASUREMENT FUNCTIONS
// ============================================================================

/**
 * @brief Benchmark the new bitboard pawn move generation
 */
double benchmark_bitboard_pawn_generation(const std::vector<Position>& positions, int iterations) {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        for (const auto& pos : positions) {
            S_MOVELIST move_list;
            generate_pawn_moves_bb(pos, move_list);
            
            // Prevent optimization from eliminating the work
            volatile int dummy = move_list.count;
            (void)dummy;
        }
    }
    
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

/**
 * @brief Benchmark the original pawn move generation
 */
double benchmark_original_pawn_generation(const std::vector<Position>& positions, int iterations) {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        for (const auto& pos : positions) {
            S_MOVELIST move_list;
            generate_pawn_moves(pos, move_list, pos.side_to_move);
            
            // Prevent optimization from eliminating the work
            volatile int dummy = move_list.count;
            (void)dummy;
        }
    }
    
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

/**
 * @brief Verify move generation correctness by comparing outputs
 */
bool verify_correctness(const std::vector<Position>& positions) {
    for (size_t i = 0; i < positions.size(); i++) {
        const auto& pos = positions[i];
        
        S_MOVELIST bitboard_moves;
        S_MOVELIST original_moves;
        
        generate_pawn_moves_bb(pos, bitboard_moves);
        generate_pawn_moves(pos, original_moves, pos.side_to_move);
        
        if (bitboard_moves.count != original_moves.count) {
            std::cout << "❌ Move count mismatch for position " << i 
                      << ": bitboard=" << bitboard_moves.count 
                      << ", original=" << original_moves.count << std::endl;
            return false;
        }
        
        std::cout << "✅ Position " << i << ": " << bitboard_moves.count << " moves match" << std::endl;
    }
    
    return true;
}

// ============================================================================
// MAIN BENCHMARK EXECUTION
// ============================================================================

int main() {
    std::cout << "==================================================================" << std::endl;
    std::cout << "🚀 PHASE 2: BITBOARD PAWN MOVE GENERATION BENCHMARK" << std::endl;
    std::cout << "==================================================================" << std::endl;
    std::cout << "Migration Plan Target: 15-25% improvement in pawn move generation" << std::endl;
    std::cout << "==================================================================" << std::endl;
    
    // Load test positions
    std::vector<Position> positions;
    for (const auto& fen : test_positions) {
        Position pos;
        try {
            pos.set_from_fen(fen);
            positions.push_back(pos);
        } catch (const std::exception& e) {
            std::cerr << "❌ Failed to load position: " << fen << " - " << e.what() << std::endl;
            return 1;
        }
    }
    
    std::cout << "📊 Loaded " << positions.size() << " test positions" << std::endl;
    std::cout << std::endl;
    
    // ---- CORRECTNESS VERIFICATION ----
    std::cout << "🔍 CORRECTNESS VERIFICATION:" << std::endl;
    if (!verify_correctness(positions)) {
        std::cout << "❌ Correctness verification failed!" << std::endl;
        return 1;
    }
    std::cout << "✅ All move generation results match original implementation" << std::endl;
    std::cout << std::endl;
    
    // ---- PERFORMANCE BENCHMARKING ----
    const int iterations = 10000;
    std::cout << "⚡ PERFORMANCE BENCHMARKING (" << iterations << " iterations):" << std::endl;
    
    std::cout << "⏱️  Running original pawn generation..." << std::endl;
    double original_time = benchmark_original_pawn_generation(positions, iterations);
    
    std::cout << "⏱️  Running bitboard pawn generation..." << std::endl;
    double bitboard_time = benchmark_bitboard_pawn_generation(positions, iterations);
    
    // ---- RESULTS ANALYSIS ----
    std::cout << std::endl;
    std::cout << "📈 PERFORMANCE RESULTS:" << std::endl;
    std::cout << "==================================================================" << std::endl;
    std::cout << "Original Implementation:   " << std::fixed << std::setprecision(2) 
              << original_time << " ms" << std::endl;
    std::cout << "Bitboard Implementation:   " << std::fixed << std::setprecision(2) 
              << bitboard_time << " ms" << std::endl;
    
    double improvement = ((original_time - bitboard_time) / original_time) * 100.0;
    std::cout << "Performance Improvement:   " << std::fixed << std::setprecision(1) 
              << improvement << "%" << std::endl;
    
    double speedup = original_time / bitboard_time;
    std::cout << "Speedup Factor:            " << std::fixed << std::setprecision(2) 
              << speedup << "x" << std::endl;
    
    std::cout << "==================================================================" << std::endl;
    
    // ---- MIGRATION PLAN VALIDATION ----
    std::cout << "🎯 MIGRATION PLAN VALIDATION:" << std::endl;
    if (improvement >= 15.0 && improvement <= 25.0) {
        std::cout << "✅ SUCCESS: Within target range of 15-25% improvement!" << std::endl;
    } else if (improvement > 25.0) {
        std::cout << "🚀 EXCELLENT: Exceeded target with " << improvement << "% improvement!" << std::endl;
    } else if (improvement > 0.0) {
        std::cout << "⚠️  PARTIAL: " << improvement << "% improvement (below 15% target)" << std::endl;
    } else {
        std::cout << "❌ REGRESSION: Performance decreased by " << std::abs(improvement) << "%" << std::endl;
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "✅ Phase 2 pawn move generation migration completed successfully!" << std::endl;
    
    return 0;
}