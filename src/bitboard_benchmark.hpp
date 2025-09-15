/**
 * @file bitboard_benchmark.hpp
 * @brief Performance comparison framework for piece list vs bitboard implementations
 * 
 * This module provides utilities to measure and compare the performance of different
 * move generation approaches during the migration from piece lists to bitboards.
 * Essential for tracking progress and identifying performance regressions.
 * 
 * ## Benchmark Categories
 * - **Attack Generation**: Compare piece list vs bitboard attack calculation
 * - **Move Generation**: Time difference for complete move generation
 * - **Memory Usage**: Compare cache performance and memory access patterns
 * 
 * ## Usage Pattern
 * ```cpp
 * BenchmarkResults results = run_attack_benchmark(position, 1000000);
 * std::cout << "Piece List: " << results.piece_list_time_ns << " ns" << std::endl;
 * std::cout << "Bitboard: " << results.bitboard_time_ns << " ns" << std::endl;
 * std::cout << "Speedup: " << results.speedup_factor << "x" << std::endl;
 * ```
 * 
 * @author MTDuke71
 * @version 1.0
 * @see BITBOARD_MIGRATION_PLAN.md for integration with migration strategy
 */

#pragma once

#include "position.hpp"
#include "chess_types.hpp"
#include <chrono>
#include <iostream>

namespace BitboardBenchmark {

/**
 * @brief Results from a performance comparison benchmark
 */
struct BenchmarkResults {
    double piece_list_time_ns;    // Time for piece list approach (nanoseconds)
    double bitboard_time_ns;      // Time for bitboard approach (nanoseconds)
    double speedup_factor;        // Bitboard speedup (positive = faster, negative = slower)
    size_t iterations;            // Number of iterations performed
    bool correctness_match;       // Whether both approaches produced identical results
};

/**
 * @brief High-precision timer for micro-benchmarking
 */
class PrecisionTimer {
public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }
    
    void stop() {
        end_time = std::chrono::high_resolution_clock::now();
    }
    
    double elapsed_nanoseconds() const {
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
        return duration.count();
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
};

/**
 * @brief Benchmark knight attack generation: piece list vs bitboard lookup
 * @param pos Test position for benchmarking
 * @param iterations Number of iterations to run
 * @return Performance comparison results
 */
BenchmarkResults benchmark_knight_attacks(const Position& pos, size_t iterations = 100000);

/**
 * @brief Benchmark pawn attack generation: piece list vs bitboard lookup
 * @param pos Test position for benchmarking  
 * @param iterations Number of iterations to run
 * @return Performance comparison results
 */
BenchmarkResults benchmark_pawn_attacks(const Position& pos, size_t iterations = 100000);

/**
 * @brief Benchmark bishop attack generation: piece list vs bitboard
 * @param pos Test position for benchmarking
 * @param iterations Number of iterations to run
 * @return Performance comparison results
 */
BenchmarkResults benchmark_bishop_attacks(const Position& pos, size_t iterations = 10000);

/**
 * @brief Benchmark rook attack generation: piece list vs bitboard
 * @param pos Test position for benchmarking
 * @param iterations Number of iterations to run  
 * @return Performance comparison results
 */
BenchmarkResults benchmark_rook_attacks(const Position& pos, size_t iterations = 10000);

/**
 * @brief Run comprehensive benchmark suite comparing all piece types
 * @param pos Test position for benchmarking
 * @return Summary of all benchmark results
 */
void run_comprehensive_benchmark(const Position& pos);

/**
 * @brief Print benchmark results in a formatted table
 * @param results The benchmark results to display
 * @param piece_name Name of the piece type being benchmarked
 */
void print_benchmark_results(const BenchmarkResults& results, const std::string& piece_name);

/**
 * @brief Verify that piece list and bitboard approaches produce identical attack sets
 * @param pos Test position
 * @param piece_type Piece type to verify
 * @param color Color of pieces to verify
 * @return true if both approaches produce identical results
 */
bool verify_attack_correctness(const Position& pos, PieceType piece_type, Color color);

} // namespace BitboardBenchmark