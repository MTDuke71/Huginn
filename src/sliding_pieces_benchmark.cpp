/**
 * @file sliding_pieces_benchmark.cpp
 * @brief Comprehensive benchmark for sliding piece move generation
 * 
 * Tests bishop, rook, and queen move generation using pure bitboard architecture.
 * Validates performance and correctness of magic bitboard implementation.
 * 
 * @author MTDuke71
 * @version 1.0
 */

#include "bitboard_movegen_pure.hpp"
#include "bitboard_position.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

using namespace BitboardMoveGen;

// ============================================================================
// SLIDING PIECE SPECIFIC BENCHMARKS
// ============================================================================

/**
 * @brief Benchmark bishop move generation performance
 */
struct BishopBenchmarkResult {
    double moves_per_second;
    size_t total_moves;
    double elapsed_ms;
    
    void print() const {
        std::cout << "Bishop Move Generation:" << std::endl;
        std::cout << "  Total moves: " << total_moves << std::endl;
        std::cout << "  Elapsed time: " << elapsed_ms << " ms" << std::endl;
        std::cout << "  Performance: " << moves_per_second / 1000000.0 << " M moves/sec" << std::endl;
    }
};

BishopBenchmarkResult benchmark_bishop_moves(const std::vector<BitboardPosition>& positions, int iterations) {
    BitboardMoveList moves;
    size_t total_moves = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        for (const auto& pos : positions) {
            moves.clear();
            generate_bishop_moves(pos, moves);
            total_moves += moves.moves.size();
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double elapsed_ms = duration.count() / 1000.0;
    double moves_per_second = (total_moves * 1000000.0) / duration.count();
    
    return {moves_per_second, total_moves, elapsed_ms};
}

/**
 * @brief Benchmark rook move generation performance
 */
struct RookBenchmarkResult {
    double moves_per_second;
    size_t total_moves;
    double elapsed_ms;
    
    void print() const {
        std::cout << "Rook Move Generation:" << std::endl;
        std::cout << "  Total moves: " << total_moves << std::endl;
        std::cout << "  Elapsed time: " << elapsed_ms << " ms" << std::endl;
        std::cout << "  Performance: " << moves_per_second / 1000000.0 << " M moves/sec" << std::endl;
    }
};

RookBenchmarkResult benchmark_rook_moves(const std::vector<BitboardPosition>& positions, int iterations) {
    BitboardMoveList moves;
    size_t total_moves = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        for (const auto& pos : positions) {
            moves.clear();
            generate_rook_moves(pos, moves);
            total_moves += moves.moves.size();
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double elapsed_ms = duration.count() / 1000.0;
    double moves_per_second = (total_moves * 1000000.0) / duration.count();
    
    return {moves_per_second, total_moves, elapsed_ms};
}

/**
 * @brief Benchmark queen move generation performance
 */
struct QueenBenchmarkResult {
    double moves_per_second;
    size_t total_moves;
    double elapsed_ms;
    
    void print() const {
        std::cout << "Queen Move Generation:" << std::endl;
        std::cout << "  Total moves: " << total_moves << std::endl;
        std::cout << "  Elapsed time: " << elapsed_ms << " ms" << std::endl;
        std::cout << "  Performance: " << moves_per_second / 1000000.0 << " M moves/sec" << std::endl;
    }
};

QueenBenchmarkResult benchmark_queen_moves(const std::vector<BitboardPosition>& positions, int iterations) {
    BitboardMoveList moves;
    size_t total_moves = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        for (const auto& pos : positions) {
            moves.clear();
            generate_queen_moves(pos, moves);
            total_moves += moves.moves.size();
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double elapsed_ms = duration.count() / 1000.0;
    double moves_per_second = (total_moves * 1000000.0) / duration.count();
    
    return {moves_per_second, total_moves, elapsed_ms};
}

// ============================================================================
// COMPLETE SLIDING PIECE BENCHMARK
// ============================================================================

struct SlidingPiecesBenchmarkResult {
    BishopBenchmarkResult bishops;
    RookBenchmarkResult rooks;
    QueenBenchmarkResult queens;
    double combined_performance;
    
    void print() const {
        std::cout << "========================================" << std::endl;
        std::cout << "     SLIDING PIECES BENCHMARK RESULTS" << std::endl;
        std::cout << "========================================" << std::endl;
        
        bishops.print();
        std::cout << std::endl;
        
        rooks.print();
        std::cout << std::endl;
        
        queens.print();
        std::cout << std::endl;
        
        std::cout << "Combined Performance: " << combined_performance / 1000000.0 << " M moves/sec" << std::endl;
        
        // Performance assessment
        if (combined_performance >= 4000000) {
            std::cout << "Assessment: EXCELLENT - High-performance sliding piece generation" << std::endl;
        } else if (combined_performance >= 2000000) {
            std::cout << "Assessment: GOOD - Solid sliding piece performance" << std::endl;
        } else if (combined_performance >= 1000000) {
            std::cout << "Assessment: MODERATE - Acceptable sliding piece performance" << std::endl;
        } else {
            std::cout << "Assessment: POOR - Sliding piece performance needs improvement" << std::endl;
        }
    }
};

// ============================================================================
// MAIN BENCHMARK ENTRY POINT
// ============================================================================

int main() {
    std::cout << "Sliding Pieces Benchmark - Pure 64-square Bitboard Architecture" << std::endl;
    std::cout << "================================================================" << std::endl;
    
    // Test positions with varying piece configurations
    std::vector<BitboardPosition> test_positions;
    
    // Starting position
    BitboardPosition start_pos;
    start_pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    test_positions.push_back(start_pos);
    
    // Middle game position with many pieces
    BitboardPosition middle_game;
    middle_game.set_from_fen("r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
    test_positions.push_back(middle_game);
    
    // Endgame with few pieces
    BitboardPosition endgame;
    endgame.set_from_fen("8/8/8/3k4/8/8/3K4/1R1Q4 w - - 0 1");
    test_positions.push_back(endgame);
    
    // Position with centralized pieces
    BitboardPosition central;
    central.set_from_fen("8/8/8/3QRB2/3BRQ2/8/8/8 w - - 0 1");
    test_positions.push_back(central);
    
    // Complex tactical position
    BitboardPosition tactical;
    tactical.set_from_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    test_positions.push_back(tactical);
    
    const int iterations = 50000;
    
    std::cout << "Testing with " << test_positions.size() << " positions, " 
              << iterations << " iterations each..." << std::endl << std::endl;
    
    // Run individual sliding piece benchmarks
    auto bishop_result = benchmark_bishop_moves(test_positions, iterations);
    auto rook_result = benchmark_rook_moves(test_positions, iterations);
    auto queen_result = benchmark_queen_moves(test_positions, iterations);
    
    // Calculate combined performance
    double total_moves = bishop_result.total_moves + rook_result.total_moves + queen_result.total_moves;
    double total_time_us = (bishop_result.elapsed_ms + rook_result.elapsed_ms + queen_result.elapsed_ms) * 1000.0;
    double combined_performance = (total_moves * 1000000.0) / total_time_us;
    
    // Display results
    SlidingPiecesBenchmarkResult results = {
        bishop_result, rook_result, queen_result, combined_performance
    };
    
    results.print();
    
    return 0;
}