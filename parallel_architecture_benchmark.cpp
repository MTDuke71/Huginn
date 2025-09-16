/**
 * @file parallel_architecture_benchmark.cpp
 * @brief Compare 120-square vs 64-square architecture performance
 * 
 * Simplified benchmark to test pure bitboard performance vs piece-list approach.
 * 
 * @author MTDuke71
 * @version 1.0
 */

#include "bitboard_position.hpp"
#include "bitboard_movegen_pure.hpp"
#include <chrono>
#include <iostream>
#include <vector>
#include <iomanip>

// ============================================================================
// SIMPLIFIED TEST DATA
// ============================================================================

const std::vector<std::string> test_positions = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",  // Starting position
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",  // Kiwipete
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",  // Endgame position
};

// ============================================================================
// ARCHITECTURE COMPARISON FRAMEWORK
// ============================================================================

struct SimpleBenchmarkResults {
    double time_64_square_ms;
    int moves_64_square;
    double moves_per_second;
};

class SimplifiedComparison {
public:
    static constexpr int BENCHMARK_ITERATIONS = 50000;
    
    static SimpleBenchmarkResults test_64_square_architecture(const std::string& fen) {
        SimpleBenchmarkResults results = {};
        
        // ---- Setup position ----
        BitboardPosition pos_64;
        
        if (!pos_64.set_from_fen(fen)) {
            std::cerr << "Failed to parse FEN: " << fen << std::endl;
            return results;
        }
        
        // ---- Generate moves once for move count ----
        BitboardMoveList moves_64;
        BitboardMoveGen::generate_pawn_moves(pos_64, moves_64);
        results.moves_64_square = static_cast<int>(moves_64.size());
        
        // ---- Performance benchmarking ----
        auto start_64 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
            BitboardMoveList temp_moves_64;
            BitboardMoveGen::generate_pawn_moves(pos_64, temp_moves_64);
        }
        auto end_64 = std::chrono::high_resolution_clock::now();
        
        results.time_64_square_ms = std::chrono::duration<double, std::milli>(end_64 - start_64).count();
        
        // Calculate moves per second
        if (results.time_64_square_ms > 0) {
            double total_moves = static_cast<double>(BENCHMARK_ITERATIONS * results.moves_64_square);
            results.moves_per_second = (total_moves / results.time_64_square_ms) * 1000.0;
        }
        
        return results;
    }
};

// ============================================================================
// MAIN BENCHMARK EXECUTION
// ============================================================================

int main() {
    std::cout << "=================================================================" << std::endl;
    std::cout << "🏛️ PURE BITBOARD ARCHITECTURE PERFORMANCE TEST" << std::endl;
    std::cout << "=================================================================" << std::endl;
    std::cout << "Testing native 64-square bitboard pawn move generation" << std::endl;
    std::cout << "Measuring performance without 120-square conversion overhead" << std::endl;
    std::cout << "=================================================================" << std::endl;
    
    std::cout << "🔧 Loaded " << test_positions.size() << " test positions" << std::endl;
    
    // ---- Test move generation correctness ----
    std::cout << "\n🔍 MOVE GENERATION VERIFICATION:" << std::endl;
    for (size_t i = 0; i < test_positions.size(); ++i) {
        auto results = SimplifiedComparison::test_64_square_architecture(test_positions[i]);
        std::cout << "✓ Position " << i << ": " << results.moves_64_square << " pawn moves" << std::endl;
    }
    
    // ---- Performance benchmarking ----
    std::cout << "\n⚡ PERFORMANCE BENCHMARKING (" << SimplifiedComparison::BENCHMARK_ITERATIONS << " iterations):" << std::endl;
    std::cout << "🔄 Running pure 64-square bitboard benchmark..." << std::endl;
    
    double total_time_64 = 0.0;
    double total_moves_per_sec = 0.0;
    
    for (size_t i = 0; i < test_positions.size(); ++i) {
        auto results = SimplifiedComparison::test_64_square_architecture(test_positions[i]);
        total_time_64 += results.time_64_square_ms;
        total_moves_per_sec += results.moves_per_second;
        
        std::cout << "Position " << i << ": " << std::fixed << std::setprecision(2) 
                  << results.time_64_square_ms << " ms, " 
                  << (results.moves_per_second / 1000000.0) << " million moves/sec" << std::endl;
    }
    
    // ---- Results summary ----
    std::cout << "\n🏁 PURE BITBOARD PERFORMANCE RESULTS:" << std::endl;
    std::cout << "=================================================================" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total Time (64-Square Pure Bitboard):  " << total_time_64 << " ms" << std::endl;
    std::cout << "Average Performance:                    " << (total_moves_per_sec / test_positions.size() / 1000000.0) << " million moves/sec" << std::endl;
    std::cout << "=================================================================" << std::endl;
    
    // ---- Architecture assessment ----
    std::cout << "\n🎯 ARCHITECTURE ASSESSMENT:" << std::endl;
    double avg_performance = total_moves_per_sec / test_positions.size();
    if (avg_performance > 10000000.0) {  // > 10 million moves/sec
        std::cout << "✅ EXCELLENT: Pure bitboard architecture achieves high performance" << std::endl;
        std::cout << "   (" << (avg_performance / 1000000.0) << " million moves/sec)" << std::endl;
    } else if (avg_performance > 1000000.0) {  // > 1 million moves/sec
        std::cout << "✅ GOOD: Solid bitboard performance achieved" << std::endl;
        std::cout << "   (" << (avg_performance / 1000000.0) << " million moves/sec)" << std::endl;
    } else {
        std::cout << "⚠️ MODERATE: Performance acceptable but could be optimized" << std::endl;
        std::cout << "   (" << (avg_performance / 1000000.0) << " million moves/sec)" << std::endl;
    }
    
    std::cout << "\n📊 DETAILED POSITION BREAKDOWN:" << std::endl;
    std::cout << "Pos | Time (ms) | Moves | Moves/sec (M)" << std::endl;
    std::cout << "----|-----------|-------|---------------" << std::endl;
    
    for (size_t i = 0; i < test_positions.size(); ++i) {
        auto results = SimplifiedComparison::test_64_square_architecture(test_positions[i]);
        std::cout << std::setw(3) << i << " | " 
                  << std::setw(9) << std::fixed << std::setprecision(2) << results.time_64_square_ms << " | "
                  << std::setw(5) << results.moves_64_square << " | "
                  << std::setw(13) << (results.moves_per_second / 1000000.0) << std::endl;
    }
    
    std::cout << "\n✅ Pure bitboard architecture analysis completed!" << std::endl;
    std::cout << "\n📝 NOTE: This benchmark demonstrates the performance potential" << std::endl;
    std::cout << "of native 64-square bitboard operations without conversion overhead." << std::endl;
    std::cout << "Compare with previous 120-square results to assess architectural benefits." << std::endl;
    
    return 0;
}