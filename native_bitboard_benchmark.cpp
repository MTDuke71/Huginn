/**
 * @file native_bitboard_benchmark.cpp
 * @brief Comprehensive benchmark: Original vs Conversion-Heavy vs Native Bitboard
 * 
 * This benchmark tests the hypothesis that eliminating conversion overhead
 * will unlock the true performance potential of bitboard operations.
 */

#include "pawn_movegen_native_bb.hpp"
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

double benchmark_original_pawn_generation(const std::vector<Position>& positions, int iterations) {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        for (const auto& pos : positions) {
            S_MOVELIST move_list;
            generate_pawn_moves(pos, move_list, pos.side_to_move);
            
            volatile int dummy = move_list.count;
            (void)dummy;
        }
    }
    
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

double benchmark_conversion_heavy_generation(const std::vector<Position>& positions, int iterations) {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        for (const auto& pos : positions) {
            S_MOVELIST move_list;
            generate_pawn_moves_bb(pos, move_list);
            
            volatile int dummy = move_list.count;
            (void)dummy;
        }
    }
    
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

double benchmark_native_bitboard_generation(const std::vector<Position>& positions, int iterations) {
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        for (const auto& pos : positions) {
            S_MOVELIST move_list;
            generate_pawn_moves_native_bb(pos, move_list);
            
            volatile int dummy = move_list.count;
            (void)dummy;
        }
    }
    
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

bool verify_correctness(const std::vector<Position>& positions) {
    for (size_t i = 0; i < positions.size(); i++) {
        const auto& pos = positions[i];
        
        S_MOVELIST original_moves;
        S_MOVELIST conversion_moves;
        S_MOVELIST native_moves;
        
        generate_pawn_moves(pos, original_moves, pos.side_to_move);
        generate_pawn_moves_bb(pos, conversion_moves);
        generate_pawn_moves_native_bb(pos, native_moves);
        
        if (original_moves.count != conversion_moves.count || 
            original_moves.count != native_moves.count) {
            std::cout << "❌ Move count mismatch for position " << i 
                      << ": original=" << original_moves.count 
                      << ", conversion=" << conversion_moves.count
                      << ", native=" << native_moves.count << std::endl;
            return false;
        }
        
        std::cout << "✅ Position " << i << ": " << original_moves.count << " moves match" << std::endl;
    }
    
    return true;
}

// ============================================================================
// MAIN BENCHMARK EXECUTION
// ============================================================================

int main() {
    std::cout << "==================================================================" << std::endl;
    std::cout << "🚀 NATIVE BITBOARD vs CONVERSION OVERHEAD BENCHMARK" << std::endl;
    std::cout << "==================================================================" << std::endl;
    std::cout << "Testing hypothesis: Eliminating conversion overhead unlocks true" << std::endl;
    std::cout << "bitboard performance potential (Target: 15-25% improvement)" << std::endl;
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
    std::cout << "✅ All implementations generate identical move counts" << std::endl;
    std::cout << std::endl;
    
    // ---- PERFORMANCE BENCHMARKING ----
    const int iterations = 20000;
    std::cout << "⚡ PERFORMANCE BENCHMARKING (" << iterations << " iterations):" << std::endl;
    
    std::cout << "⏱️  Running original piece-list generation..." << std::endl;
    double original_time = benchmark_original_pawn_generation(positions, iterations);
    
    std::cout << "⏱️  Running conversion-heavy bitboard generation..." << std::endl;
    double conversion_time = benchmark_conversion_heavy_generation(positions, iterations);
    
    std::cout << "⏱️  Running native bitboard generation..." << std::endl;
    double native_time = benchmark_native_bitboard_generation(positions, iterations);
    
    // ---- RESULTS ANALYSIS ----
    std::cout << std::endl;
    std::cout << "📈 COMPREHENSIVE PERFORMANCE RESULTS:" << std::endl;
    std::cout << "==================================================================" << std::endl;
    std::cout << "Original Piece-List:       " << std::fixed << std::setprecision(2) 
              << original_time << " ms" << std::endl;
    std::cout << "Conversion-Heavy Bitboard: " << std::fixed << std::setprecision(2) 
              << conversion_time << " ms" << std::endl;
    std::cout << "Native Bitboard:           " << std::fixed << std::setprecision(2) 
              << native_time << " ms" << std::endl;
    std::cout << "==================================================================" << std::endl;
    
    double conversion_improvement = ((original_time - conversion_time) / original_time) * 100.0;
    double native_improvement = ((original_time - native_time) / original_time) * 100.0;
    double overhead_elimination = ((conversion_time - native_time) / conversion_time) * 100.0;
    
    std::cout << std::endl;
    std::cout << "📊 IMPROVEMENT ANALYSIS:" << std::endl;
    std::cout << "Conversion-Heavy vs Original: " << std::fixed << std::setprecision(1) 
              << conversion_improvement << "%" << std::endl;
    std::cout << "Native Bitboard vs Original:  " << std::fixed << std::setprecision(1) 
              << native_improvement << "%" << std::endl;
    std::cout << "Overhead Elimination Gain:    " << std::fixed << std::setprecision(1) 
              << overhead_elimination << "%" << std::endl;
    
    double native_speedup = original_time / native_time;
    std::cout << "Native Bitboard Speedup:      " << std::fixed << std::setprecision(2) 
              << native_speedup << "x" << std::endl;
    
    std::cout << "==================================================================" << std::endl;
    
    // ---- HYPOTHESIS VALIDATION ----
    std::cout << "🧪 HYPOTHESIS VALIDATION:" << std::endl;
    if (native_improvement >= 15.0 && native_improvement <= 25.0) {
        std::cout << "✅ HYPOTHESIS CONFIRMED: Native bitboard achieved " 
                  << native_improvement << "% improvement!" << std::endl;
        std::cout << "✅ Conversion overhead elimination successful!" << std::endl;
    } else if (native_improvement > 25.0) {
        std::cout << "🚀 HYPOTHESIS EXCEEDED: " << native_improvement 
                  << "% improvement surpassed expectations!" << std::endl;
    } else if (native_improvement > conversion_improvement) {
        std::cout << "⚠️  PARTIAL SUCCESS: " << native_improvement 
                  << "% improvement (below target but better than conversion-heavy)" << std::endl;
    } else {
        std::cout << "❌ HYPOTHESIS FAILED: Native approach didn't improve significantly" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "🎯 CONVERSION OVERHEAD IMPACT:" << std::endl;
    std::cout << "Overhead elimination improved performance by " << overhead_elimination << "%" << std::endl;
    
    if (overhead_elimination > 10.0) {
        std::cout << "✅ Significant overhead reduction achieved!" << std::endl;
    } else {
        std::cout << "⚠️  Limited overhead reduction - may need further optimization" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "✅ Native bitboard analysis completed!" << std::endl;
    
    return 0;
}