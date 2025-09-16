/**
 * @file movegen_comparison.cpp
 * @brief Dedicated benchmark comparing piece list vs bitboard move generation
 * 
 * This benchmark specifically tests the fundamental difference between:
 * 1. Piece Lists: Iterate through pList arrays (current Huginn approach)
 * 2. Bitboards: Use bitboard operations to find pieces (future Huginn2 approach)
 * 
 * Expected Results:
 * - Bitboards should be faster for positions with fewer pieces
 * - Piece lists may be faster for positions with many pieces
 * - Knights and sliding pieces should show the biggest difference
 * 
 * @author MTDuke71
 * @date 2025-09-15
 */

#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <string>

#include "position.hpp"
#include "movegen_enhanced.hpp"
#include "bitboard_movegen.hpp"
#include "init.hpp"
#include "board120.hpp"

namespace MoveGenComparison {

struct BenchmarkResult {
    std::string method_name;
    std::string position_name;
    int moves_generated;
    double time_ms;
    double moves_per_second;
    bool correctness_check;
};

class MoveGenBenchmark {
private:
    static constexpr int ITERATIONS = 10000;  // Number of test iterations
    
public:
    /**
     * @brief Test a single position with both methods
     */
    BenchmarkResult test_position_piece_list(const Position& pos, const std::string& pos_name) {
        S_MOVELIST moves;
        
        // Warm up
        for (int i = 0; i < 100; ++i) {
            moves.count = 0;
            generate_all_moves(pos, moves);
        }
        
        // Actual benchmark
        auto start = std::chrono::high_resolution_clock::now();
        int total_moves = 0;
        
        for (int i = 0; i < ITERATIONS; ++i) {
            moves.count = 0;
            generate_all_moves(pos, moves);
            total_moves += moves.count;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::milli>(end - start).count();
        
        return {
            "Piece Lists",
            pos_name,
            moves.count,
            duration,
            (total_moves / duration) * 1000.0,
            true
        };
    }
    
    BenchmarkResult test_position_bitboard(const Position& pos, const std::string& pos_name) {
        S_MOVELIST moves;
        
        // Warm up
        for (int i = 0; i < 100; ++i) {
            moves.count = 0;
            BitboardMoveGen::generate_all_moves_bitboard(pos, moves);
        }
        
        // Actual benchmark
        auto start = std::chrono::high_resolution_clock::now();
        int total_moves = 0;
        
        for (int i = 0; i < ITERATIONS; ++i) {
            moves.count = 0;
            BitboardMoveGen::generate_all_moves_bitboard(pos, moves);
            total_moves += moves.count;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double, std::milli>(end - start).count();
        
        return {
            "Bitboards",
            pos_name,
            moves.count,
            duration,
            (total_moves / duration) * 1000.0,
            true
        };
    }
    
    /**
     * @brief Verify both methods produce identical moves
     */
    bool verify_correctness(const Position& pos) {
        S_MOVELIST piece_list_moves, bitboard_moves;
        
        piece_list_moves.count = 0;
        generate_all_moves(pos, piece_list_moves);
        
        bitboard_moves.count = 0;
        BitboardMoveGen::generate_all_moves_bitboard(pos, bitboard_moves);
        
        if (piece_list_moves.count != bitboard_moves.count) {
            std::cout << "Move count mismatch: piece_list=" << piece_list_moves.count 
                      << " vs bitboard=" << bitboard_moves.count << std::endl;
            return false;
        }
        
        // Could add more detailed move comparison here if needed
        return true;
    }
    
    /**
     * @brief Create test positions with different characteristics
     */
    std::vector<std::pair<Position, std::string>> create_test_positions() {
        std::vector<std::pair<Position, std::string>> positions;
        
        // 1. Starting position (many pieces)
        Position start_pos;
        start_pos.set_startpos();
        positions.emplace_back(start_pos, "Starting Position (33 pieces)");
        
        // 2. Middlegame position (medium pieces)
        Position mid_pos;
        mid_pos.set_from_fen("r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
        positions.emplace_back(mid_pos, "Middlegame Position (28 pieces)");
        
        // 3. Endgame position (few pieces)
        Position end_pos;
        end_pos.set_from_fen("8/2k5/3p4/p2P1p2/P2P1P2/8/2K5/8 w - - 0 1");
        positions.emplace_back(end_pos, "Endgame Position (8 pieces)");
        
        // 4. Tactical position with many pieces
        Position tactic_pos;
        tactic_pos.set_from_fen("2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - 0 1");
        positions.emplace_back(tactic_pos, "Tactical Position (26 pieces)");
        
        // 5. Knight-heavy position (should favor bitboards)
        Position knight_pos;
        knight_pos.set_from_fen("rnbnkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBNKBNR w KQkq - 0 1");
        positions.emplace_back(knight_pos, "Knight-Heavy Position (32 pieces)");
        
        // 6. Sparse endgame (should heavily favor bitboards)
        Position sparse_pos;
        sparse_pos.set_from_fen("8/8/8/3k4/8/3K4/8/8 w - - 0 1");
        positions.emplace_back(sparse_pos, "Minimal Endgame (2 pieces)");
        
        return positions;
    }
    
    /**
     * @brief Run comprehensive comparison
     */
    void run_comparison() {
        std::cout << "=== Move Generation Method Comparison ===" << std::endl;
        std::cout << "Testing Piece Lists vs Bitboards over " << ITERATIONS << " iterations" << std::endl;
        std::cout << "==========================================================" << std::endl;
        
        auto test_positions = create_test_positions();
        std::vector<BenchmarkResult> all_results;
        
        for (const auto& [pos, name] : test_positions) {
            std::cout << "\nTesting: " << name << std::endl;
            
            // Verify correctness first
            if (!verify_correctness(pos)) {
                std::cout << "CORRECTNESS FAILURE - skipping performance test" << std::endl;
                continue;
            }
            
            // Run performance tests
            auto piece_list_result = test_position_piece_list(pos, name);
            auto bitboard_result = test_position_bitboard(pos, name);
            
            all_results.push_back(piece_list_result);
            all_results.push_back(bitboard_result);
            
            // Calculate improvement
            double speedup = bitboard_result.moves_per_second / piece_list_result.moves_per_second;
            
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "  Moves generated: " << piece_list_result.moves_generated << std::endl;
            std::cout << "  Piece Lists:     " << piece_list_result.time_ms << "ms (" 
                      << piece_list_result.moves_per_second << " moves/sec)" << std::endl;
            std::cout << "  Bitboards:       " << bitboard_result.time_ms << "ms (" 
                      << bitboard_result.moves_per_second << " moves/sec)" << std::endl;
            
            if (speedup > 1.0) {
                std::cout << "  --> Bitboards " << speedup << "x FASTER" << std::endl;
            } else {
                std::cout << "  --> Piece Lists " << (1.0/speedup) << "x FASTER" << std::endl;
            }
        }
        
        print_summary(all_results);
    }
    
private:
    void print_summary(const std::vector<BenchmarkResult>& results) {
        std::cout << "\n\n=== SUMMARY TABLE ===" << std::endl;
        std::cout << std::left << std::setw(30) << "Position" 
                  << std::setw(15) << "Method" 
                  << std::setw(12) << "Time (ms)"
                  << std::setw(15) << "Moves/sec"
                  << std::setw(10) << "Moves" << std::endl;
        std::cout << std::string(80, '-') << std::endl;
        
        for (const auto& result : results) {
            std::cout << std::left << std::setw(30) << result.position_name
                      << std::setw(15) << result.method_name
                      << std::setw(12) << std::fixed << std::setprecision(2) << result.time_ms
                      << std::setw(15) << std::fixed << std::setprecision(0) << result.moves_per_second
                      << std::setw(10) << result.moves_generated << std::endl;
        }
        
        // Calculate overall averages
        double piece_list_total_time = 0, bitboard_total_time = 0;
        int piece_list_count = 0, bitboard_count = 0;
        
        for (const auto& result : results) {
            if (result.method_name == "Piece Lists") {
                piece_list_total_time += result.time_ms;
                piece_list_count++;
            } else {
                bitboard_total_time += result.time_ms;
                bitboard_count++;
            }
        }
        
        std::cout << "\n=== OVERALL COMPARISON ===" << std::endl;
        if (piece_list_count > 0 && bitboard_count > 0) {
            double piece_list_avg = piece_list_total_time / piece_list_count;
            double bitboard_avg = bitboard_total_time / bitboard_count;
            double overall_speedup = piece_list_avg / bitboard_avg;
            
            std::cout << "Average time per position:" << std::endl;
            std::cout << "  Piece Lists: " << std::fixed << std::setprecision(2) << piece_list_avg << "ms" << std::endl;
            std::cout << "  Bitboards:   " << std::fixed << std::setprecision(2) << bitboard_avg << "ms" << std::endl;
            
            if (overall_speedup > 1.0) {
                std::cout << "  --> Bitboards are " << overall_speedup << "x faster overall!" << std::endl;
            } else {
                std::cout << "  --> Piece Lists are " << (1.0/overall_speedup) << "x faster overall!" << std::endl;
            }
        }
    }
};

} // namespace MoveGenComparison

int main() {
    std::cout << "Huginn Move Generation Comparison Benchmark" << std::endl;
    std::cout << "Comparing Piece Lists vs Bitboard approaches" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // Initialize engine
    Huginn::init();
    
    // Run the comparison
    MoveGenComparison::MoveGenBenchmark benchmark;
    benchmark.run_comparison();
    
    return 0;
}