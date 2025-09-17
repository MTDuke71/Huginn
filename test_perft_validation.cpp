/**
 * @file test_perft_validation.cpp
 * @brief Comprehensive perft validation for Phase 1 optimization
 * 
 * Tests perft depths 1-6 for starting position and depths 1-5 for Kiwipete
 * to validate complete functional correctness before Phase 2.
 * 
 * @author MTDuke71
 * @version 1.0
 */

#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <string>
#include "src/bitboard.hpp"
#include "src/bitboard_position.hpp"
#include "src/bitboard_movegen_pure.hpp"
#include "src/bitboard_perft_optimized.hpp"
#include "src/init.hpp"

using namespace std;
using namespace std::chrono;

struct PerftResult {
    uint64_t nodes;
    double time_ms;
    bool matches_expected;
};

struct PerftTestPosition {
    string name;
    string fen;
    vector<uint64_t> expected_results; // Expected nodes for depths 1, 2, 3, 4, 5, 6
    int max_depth;
};

// Known correct perft results
static const PerftTestPosition TEST_POSITIONS[] = {
    {
        "Starting Position",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        {20, 400, 8902, 197281, 4865609, 119060324}, // Depths 1-6
        6
    },
    {
        "Kiwipete",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        {48, 2039, 97862, 4085603, 193690690}, // Depths 1-5
        5
    }
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

// Recursive perft implementation using optimized move generation
uint64_t perft_recursive_optimized(BitboardPosition& pos, int depth) {
    if (depth == 0) {
        return 1;
    }
    
    if (depth == 1) {
        // At depth 1, just count legal moves
        BitboardMoveList moves;
        BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);
        return moves.moves.size();
    }
    
    BitboardMoveList moves;
    BitboardPerftOptimized::generate_legal_moves_fast(pos, moves);
    
    uint64_t total_nodes = 0;
    
    for (const auto& move : moves.moves) {
        // Convert to SimpleBitboardMove and make the move
        SimpleBitboardMove simple_move(move.from_64, move.to_64, move.promotion_type);
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        
        BitboardPosition new_pos = pos;
        if (new_pos.make_move(simple_move)) {
            total_nodes += perft_recursive_optimized(new_pos, depth - 1);
        }
    }
    
    return total_nodes;
}

// Recursive perft implementation using original move generation
uint64_t perft_recursive_original(BitboardPosition pos, int depth) {
    if (depth == 0) {
        return 1;
    }
    
    if (depth == 1) {
        // At depth 1, just count legal moves
        BitboardMoveList moves;
        BitboardMoveGen::generate_legal_moves(pos, moves);
        return moves.moves.size();
    }
    
    BitboardMoveList moves;
    BitboardMoveGen::generate_legal_moves(pos, moves);
    
    uint64_t total_nodes = 0;
    
    for (const auto& move : moves.moves) {
        // Convert to SimpleBitboardMove and make the move
        SimpleBitboardMove simple_move(move.from_64, move.to_64, move.promotion_type);
        simple_move.is_capture = move.is_capture;
        simple_move.is_ep_capture = move.is_ep_capture;
        simple_move.is_castling = move.is_castling;
        simple_move.is_promotion = move.is_promotion;
        
        BitboardPosition new_pos = pos;
        if (new_pos.make_move(simple_move)) {
            total_nodes += perft_recursive_original(new_pos, depth - 1);
        }
    }
    
    return total_nodes;
}

PerftResult run_perft_test(const PerftTestPosition& test_pos, int depth, bool use_optimized) {
    BitboardPosition pos;
    if (!pos.set_from_fen(test_pos.fen)) {
        cout << "ERROR: Failed to parse FEN: " << test_pos.fen << endl;
        return {0, 0, false};
    }
    
    PerformanceTimer timer;
    timer.start();
    
    uint64_t nodes;
    if (use_optimized) {
        BitboardPosition pos_copy = pos; // Make a copy for the recursive function
        nodes = perft_recursive_optimized(pos_copy, depth);
    } else {
        nodes = perft_recursive_original(pos, depth);
    }
    
    double time_ms = timer.elapsed_ms();
    
    uint64_t expected = test_pos.expected_results[depth - 1];
    bool matches = (nodes == expected);
    
    return {nodes, time_ms, matches};
}

void print_perft_result(const string& method, int depth, const PerftResult& result, uint64_t expected) {
    cout << "  " << method << " Depth " << depth << ": " 
         << setw(12) << result.nodes << " nodes in " 
         << setw(8) << fixed << setprecision(2) << result.time_ms << " ms";
    
    if (result.matches_expected) {
        cout << " ✓";
    } else {
        cout << " ✗ (expected " << expected << ")";
    }
    cout << endl;
}

void test_position_perft(const PerftTestPosition& test_pos) {
    cout << "\n=== " << test_pos.name << " ===" << endl;
    cout << "FEN: " << test_pos.fen << endl;
    
    bool all_passed = true;
    
    for (int depth = 1; depth <= test_pos.max_depth; depth++) {
        cout << "\nDepth " << depth << " (Expected: " << test_pos.expected_results[depth - 1] << ")" << endl;
        
        // Test original method
        PerftResult original_result = run_perft_test(test_pos, depth, false);
        print_perft_result("Original ", depth, original_result, test_pos.expected_results[depth - 1]);
        
        // Test optimized method  
        PerftResult optimized_result = run_perft_test(test_pos, depth, true);
        print_perft_result("Optimized", depth, optimized_result, test_pos.expected_results[depth - 1]);
        
        // Check results
        if (!original_result.matches_expected || !optimized_result.matches_expected) {
            all_passed = false;
        }
        
        if (original_result.matches_expected && optimized_result.matches_expected) {
            double speedup = original_result.time_ms / optimized_result.time_ms;
            cout << "  Speedup: " << fixed << setprecision(2) << speedup << "x" << endl;
        }
        
        // Stop if we hit errors at shallow depths
        if (!original_result.matches_expected || !optimized_result.matches_expected) {
            cout << "  ERROR: Stopping at depth " << depth << " due to incorrect results" << endl;
            break;
        }
    }
    
    if (all_passed) {
        cout << "\n✓ ALL DEPTHS PASSED for " << test_pos.name << endl;
    } else {
        cout << "\n✗ SOME DEPTHS FAILED for " << test_pos.name << endl;
    }
}

void run_quick_validation() {
    cout << "\n=== Quick Validation (Depth 1) ===" << endl;
    
    for (const auto& test_pos : TEST_POSITIONS) {
        BitboardPosition pos;
        if (!pos.set_from_fen(test_pos.fen)) {
            cout << "ERROR: Failed to parse " << test_pos.name << endl;
            continue;
        }
        
        // Generate moves with both methods
        BitboardMoveList original_moves, optimized_moves;
        
        BitboardPosition pos_copy = pos;
        BitboardMoveGen::generate_legal_moves(pos_copy, original_moves);
        BitboardPerftOptimized::generate_legal_moves_fast(pos, optimized_moves);
        
        cout << test_pos.name << ":" << endl;
        cout << "  Original:  " << setw(3) << original_moves.moves.size() << " moves" << endl;
        cout << "  Optimized: " << setw(3) << optimized_moves.moves.size() << " moves" << endl;
        cout << "  Expected:  " << setw(3) << test_pos.expected_results[0] << " moves" << endl;
        
        if (original_moves.moves.size() == test_pos.expected_results[0] && 
            optimized_moves.moves.size() == test_pos.expected_results[0]) {
            cout << "  Status: ✓ PASSED" << endl;
        } else {
            cout << "  Status: ✗ FAILED" << endl;
        }
        cout << endl;
    }
}

void print_summary() {
    cout << "\n=== Summary ===" << endl;
    cout << "This test validates the Phase 1 optimization by running comprehensive" << endl;
    cout << "perft tests on standard positions with known correct results." << endl;
    cout << endl;
    cout << "✓ If all depths pass: Phase 1 optimization is functionally correct" << endl;
    cout << "✓ Speedup measurements show performance improvement" << endl;
    cout << "✓ Ready to proceed to Phase 2 (magic bitboards)" << endl;
    cout << endl;
    cout << "Expected Results:" << endl;
    cout << "- Starting Position: 20, 400, 8902, 197281, 4865609, 119060324" << endl;
    cout << "- Kiwipete: 48, 2039, 97862, 4085603, 193690690" << endl;
}

int main() {
    cout << "Comprehensive Perft Validation for Phase 1 Optimization" << endl;
    cout << "=======================================================" << endl;
    
    try {
        // Initialize engine
        Huginn::init();
        if (!Huginn::is_initialized()) {
            cout << "ERROR: Failed to initialize engine" << endl;
            return 1;
        }
        
        print_summary();
        
        // Quick validation first
        run_quick_validation();
        
        // Full perft tests
        for (const auto& test_pos : TEST_POSITIONS) {
            test_position_perft(test_pos);
        }
        
        cout << "\n=== Final Status ===" << endl;
        cout << "Phase 1 optimization validation complete." << endl;
        cout << "Review results above to confirm functional correctness." << endl;
        
    } catch (const exception& e) {
        cout << "Exception caught: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}