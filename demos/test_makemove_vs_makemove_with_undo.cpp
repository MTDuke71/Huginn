// Test program comparing VICE MakeMove/TakeMove vs make_move_with_undo/undo_move performance
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include "../src/position.hpp"
#include "../src/movegen_enhanced.hpp"

class PerformanceBenchmark {
public:
    static constexpr int NUM_ITERATIONS = 100000;
    
public:
    // Test VICE Tutorial Video #41/#42: MakeMove/TakeMove approach
    static double benchmark_vice_makemove_takemove(Position& pos) {
        S_MOVELIST moves;
        generate_legal_moves_enhanced(pos, moves);
        
        if (moves.count == 0) return 0.0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_ITERATIONS; ++i) {
            // Use different moves to avoid caching effects
            S_MOVE move = moves.moves[i % moves.count];
            
            // VICE approach: MakeMove returns 1/0, TakeMove undoes
            if (pos.MakeMove(move) == 1) {
                pos.TakeMove();
            }
            // If MakeMove returned 0, it already undid the move automatically
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        return duration.count() / 1000000.0; // Convert to milliseconds
    }
    
    // Test current Huginn approach: make_move_with_undo/undo_move
    static double benchmark_huginn_makemove_with_undo(Position& pos) {
        S_MOVELIST moves;
        generate_legal_moves_enhanced(pos, moves);
        
        if (moves.count == 0) return 0.0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < NUM_ITERATIONS; ++i) {
            // Use different moves to avoid caching effects
            S_MOVE move = moves.moves[i % moves.count];
            
            // Huginn approach: make_move_with_undo then undo_move
            pos.make_move_with_undo(move);
            pos.undo_move();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        return duration.count() / 1000000.0; // Convert to milliseconds
    }
    
    // Test position integrity after both approaches
    static bool test_position_integrity() {
        Position pos1, pos2, original;
        
        // Test multiple positions
        std::vector<std::string> test_positions = {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", // Starting position
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", // Complex position
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", // Endgame position
            "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", // Castling test position
        };
        
        for (const auto& fen : test_positions) {
            pos1.set_from_fen(fen);
            pos2.set_from_fen(fen);
            original.set_from_fen(fen);
            
            S_MOVELIST moves;
            generate_legal_moves_enhanced(pos1, moves);
            
            for (int i = 0; i < std::min(10, moves.count); ++i) {
                S_MOVE move = moves.moves[i];
                
                // Test VICE approach
                pos1.set_from_fen(fen); // Reset
                if (pos1.MakeMove(move) == 1) {
                    pos1.TakeMove();
                }
                
                // Test Huginn approach
                pos2.set_from_fen(fen); // Reset
                pos2.make_move_with_undo(move);
                pos2.undo_move();
                
                // Both should match original
                if (pos1.to_fen() != original.to_fen() || pos2.to_fen() != original.to_fen()) {
                    std::cout << "INTEGRITY ERROR on move " << i << " in position: " << fen << std::endl;
                    return false;
                }
            }
        }
        
        return true;
    }
};

int main() {
    std::cout << "=== VICE MakeMove/TakeMove vs Huginn make_move_with_undo Performance Comparison ===" << std::endl;
    std::cout << "Testing " << PerformanceBenchmark::NUM_ITERATIONS << " move/undo cycles..." << std::endl << std::endl;
    
    // Test position integrity first
    std::cout << "Testing position integrity..." << std::endl;
    if (!PerformanceBenchmark::test_position_integrity()) {
        std::cout << "❌ INTEGRITY TEST FAILED!" << std::endl;
        return 1;
    }
    std::cout << "✅ Integrity test passed!" << std::endl << std::endl;
    
    // Test different positions for comprehensive benchmarking
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"Starting Position", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
        {"Complex Middlegame", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"},
        {"Endgame Position", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"},
        {"Castling Position", "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"}
    };
    
    double total_vice_time = 0.0;
    double total_huginn_time = 0.0;
    
    for (const auto& [name, fen] : test_cases) {
        std::cout << "Testing: " << name << std::endl;
        
        Position pos;
        pos.set_from_fen(fen);
        
        // Run benchmarks
        double vice_time = PerformanceBenchmark::benchmark_vice_makemove_takemove(pos);
        pos.set_from_fen(fen); // Reset position
        double huginn_time = PerformanceBenchmark::benchmark_huginn_makemove_with_undo(pos);
        
        total_vice_time += vice_time;
        total_huginn_time += huginn_time;
        
        std::cout << "  VICE MakeMove/TakeMove:      " << vice_time << " ms" << std::endl;
        std::cout << "  Huginn make_move_with_undo: " << huginn_time << " ms" << std::endl;
        
        double speedup = huginn_time / vice_time;
        if (speedup > 1.0) {
            std::cout << "  🏆 VICE is " << speedup << "x FASTER" << std::endl;
        } else {
            std::cout << "  🏆 Huginn is " << (1.0/speedup) << "x FASTER" << std::endl;
        }
        std::cout << std::endl;
    }
    
    std::cout << "=== OVERALL RESULTS ===" << std::endl;
    std::cout << "Total VICE time:      " << total_vice_time << " ms" << std::endl;
    std::cout << "Total Huginn time:    " << total_huginn_time << " ms" << std::endl;
    
    double overall_speedup = total_huginn_time / total_vice_time;
    if (overall_speedup > 1.0) {
        std::cout << "🏆 OVERALL WINNER: VICE MakeMove/TakeMove (" << overall_speedup << "x faster)" << std::endl;
    } else {
        std::cout << "🏆 OVERALL WINNER: Huginn make_move_with_undo (" << (1.0/overall_speedup) << "x faster)" << std::endl;
    }
    
    std::cout << std::endl << "=== RECOMMENDATIONS ===" << std::endl;
    if (overall_speedup > 1.1) {  // VICE is significantly faster
        std::cout << "✅ Recommend switching to VICE MakeMove/TakeMove approach" << std::endl;
        std::cout << "   - Replace search engine make_move_with_undo calls with MakeMove/TakeMove" << std::endl;
        std::cout << "   - Potential " << ((overall_speedup - 1.0) * 100) << "% performance improvement" << std::endl;
    } else if (overall_speedup < 0.9) {  // Huginn is significantly faster
        std::cout << "✅ Keep current Huginn make_move_with_undo approach" << std::endl;
        std::cout << "   - Current system is faster than VICE approach" << std::endl;
        std::cout << "   - Use VICE MakeMove only for move validation (UCI interface)" << std::endl;
    } else {
        std::cout << "⚖️  Performance is similar - architectural choice" << std::endl;
        std::cout << "   - Could use VICE for consistency with tutorial" << std::endl;
        std::cout << "   - Or keep Huginn for stability" << std::endl;
    }
    
    return 0;
}
