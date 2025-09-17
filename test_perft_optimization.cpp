/**
 * @file test_perft_optimization.cpp
 * @brief Test program for BitboardPosition optimization
 * 
 * This program compares the performance of the original vs optimized
 * legal move generation for BitboardPosition.
 * 
 * @author MTDuke71
 * @version 1.0
 */

#include <iostream>
#include <chrono>
#include <iomanip>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;
using namespace std::chrono;

// Test positions for performance comparison
const char* TEST_POSITIONS[] = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",  // Starting position
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", // Kiwipete
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",  // Complex endgame
};

const char* POSITION_NAMES[] = {
    "Starting Position",
    "Kiwipete",
    "Complex Endgame"
};

class PerformanceTimer {
private:
    high_resolution_clock::time_point start_time;
    
public:
    void start() {
        start_time = high_resolution_clock::now();
    }
    
    double elapsed_ms() {
        auto end_time = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
};

void test_move_generation_performance() {
    cout << "=== BitboardPosition Move Generation Performance Test ===" << endl;
    cout << "Testing original vs optimized legal move generation" << endl << endl;
    
    const int NUM_ITERATIONS = 10000;
    
    for (size_t pos_idx = 0; pos_idx < 3; pos_idx++) {
        cout << "Position: " << POSITION_NAMES[pos_idx] << endl;
        cout << "FEN: " << TEST_POSITIONS[pos_idx] << endl;
        
        // Create position
        BitboardPosition pos;
        if (!pos.set_from_fen(TEST_POSITIONS[pos_idx])) {
            cout << "ERROR: Failed to parse FEN: " << TEST_POSITIONS[pos_idx] << endl;
            continue;
        }
        
        // Test original method
        PerformanceTimer timer;
        BitboardMoveList original_moves;
        
        timer.start();
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            BitboardPosition pos_copy = pos; // Make a copy since function needs non-const
            BitboardMoveGen::generate_legal_moves(pos_copy, original_moves);
        }
        double original_time = timer.elapsed_ms();
        
        // Test optimized method  
        BitboardMoveList optimized_moves;
        
        timer.start();
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            BitboardPerftOptimized::generate_legal_moves_fast(pos, optimized_moves);
        }
        double optimized_time = timer.elapsed_ms();
        
        // Compare results
        double speedup = original_time / optimized_time;
        
        cout << "  Original moves:   " << original_moves.moves.size() << endl;
        cout << "  Optimized moves:  " << optimized_moves.moves.size() << endl;
        cout << "  Original time:    " << fixed << setprecision(2) << original_time << " ms" << endl;
        cout << "  Optimized time:   " << fixed << setprecision(2) << optimized_time << " ms" << endl;
        cout << "  Speedup:          " << fixed << setprecision(2) << speedup << "x" << endl;
        
        if (original_moves.moves.size() != optimized_moves.moves.size()) {
            cout << "  WARNING: Move count mismatch!" << endl;
        } else {
            cout << "  ✓ Move counts match" << endl;
        }
        
        cout << endl;
    }
}

void test_basic_functionality() {
    cout << "=== Basic Functionality Test ===" << endl;
    
    // Test starting position
    BitboardPosition pos;
    if (!pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        cout << "ERROR: Failed to parse starting position" << endl;
        return;
    }
    
    // Generate moves with both methods
    BitboardMoveList original_moves, optimized_moves;
    
    BitboardPosition pos_copy = pos;
    BitboardMoveGen::generate_legal_moves(pos_copy, original_moves);
    BitboardPerftOptimized::generate_legal_moves_fast(pos, optimized_moves);
    
    cout << "Starting position legal moves:" << endl;
    cout << "  Original method:  " << original_moves.moves.size() << " moves" << endl;
    cout << "  Optimized method: " << optimized_moves.moves.size() << " moves" << endl;
    
    if (original_moves.moves.size() == optimized_moves.moves.size()) {
        cout << "  ✓ Basic functionality test PASSED" << endl;
    } else {
        cout << "  ✗ Basic functionality test FAILED" << endl;
    }
    
    cout << endl;
}

void run_simple_perft_comparison(const char* fen, int depth) {
    cout << "=== Simple Perft Comparison (Depth " << depth << ") ===" << endl;
    cout << "FEN: " << fen << endl;
    
    BitboardPosition pos;
    if (!pos.set_from_fen(fen)) {
        cout << "ERROR: Failed to parse FEN" << endl;
        return;
    }
    
    PerformanceTimer timer;
    
    // For now, just show basic move generation performance
    // Actual perft would require implementing recursive perft with optimized moves
    timer.start();
    BitboardMoveList moves;
    BitboardPosition pos_copy = pos;
    BitboardMoveGen::generate_legal_moves(pos_copy, moves);
    double time_ms = timer.elapsed_ms();
    
    cout << "  Legal moves: " << moves.size() << endl;
    cout << "  Generation time: " << fixed << setprecision(2) << time_ms << " ms" << endl;
    
    cout << endl;
}

int main() {
    cout << "BitboardPosition Optimization Test Suite" << endl;
    cout << "=========================================" << endl << endl;
    
    try {
        // Initialize attack tables
        Huginn::init();
        if (!Huginn::is_initialized()) {
            cout << "ERROR: Failed to initialize engine" << endl;
            return 1;
        }
        
        // Run tests
        test_basic_functionality();
        test_move_generation_performance();
        run_simple_perft_comparison("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4);
        
        cout << "=== Test Summary ===" << endl;
        cout << "If optimized method shows 2-3x speedup, Phase 1 optimization is working!" << endl;
        cout << "Next steps: Implement magic bitboards for Phase 2 (5x improvement)" << endl;
        
    } catch (const exception& e) {
        cout << "Exception caught: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}