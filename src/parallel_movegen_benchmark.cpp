#include "parallel_movegen.hpp"
#include "movegen_enhanced.hpp"
#include "position.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>

class MoveGenBenchmark {
public:
    struct BenchmarkResult {
        std::string position_name;
        std::string method;
        int move_count;
        double time_microseconds;
        double moves_per_second;
    };
    
    static void run_comprehensive_benchmark() {
        std::cout << "=== Move Generation Multithreading Benchmark ===" << std::endl;
        std::cout << "Hardware threads: " << std::thread::hardware_concurrency() << std::endl;
        
        // Auto-configure parallel settings
        ParallelConfig::auto_configure();
        std::cout << "Parallel config - Generation: " << (ParallelConfig::use_parallel_generation ? "ON" : "OFF")
                  << ", Legal: " << (ParallelConfig::use_parallel_legal ? "ON" : "OFF")
                  << ", Threads: " << ParallelConfig::thread_count << std::endl << std::endl;
        
        // Test positions with varying complexity
        std::vector<std::pair<std::string, std::string>> test_positions = {
            {"Starting Position", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"},
            {"Kiwipete", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"},
            {"Complex Middlegame", "r1bq1rk1/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQ - 0 1"},
            {"Endgame", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"},
            {"Tactical Position", "r2qkb1r/pp2nppp/3p4/2pNN1B1/2BnP3/3P4/PPP2PPP/R2QK2R w KQkq - 0 1"}
        };
        
        std::vector<BenchmarkResult> results;
        
        for (const auto& [name, fen] : test_positions) {
            std::cout << "Testing: " << name << std::endl;
            
            Position pos;
            pos.set_from_fen(fen);
            
            // Benchmark pseudo-legal move generation
            auto sequential_pseudo = benchmark_pseudo_legal_sequential(pos);
            auto parallel_pseudo = benchmark_pseudo_legal_parallel(pos);
            
            // Benchmark legal move generation
            auto sequential_legal = benchmark_legal_sequential(pos);
            auto parallel_legal = benchmark_legal_parallel(pos);
            
            // Store results
            results.push_back({name, "Pseudo Sequential", sequential_pseudo.first, sequential_pseudo.second, 0});
            results.push_back({name, "Pseudo Parallel", parallel_pseudo.first, parallel_pseudo.second, 0});
            results.push_back({name, "Legal Sequential", sequential_legal.first, sequential_legal.second, 0});
            results.push_back({name, "Legal Parallel", parallel_legal.first, parallel_legal.second, 0});
            
            // Calculate speedup
            double pseudo_speedup = sequential_pseudo.second / parallel_pseudo.second;
            double legal_speedup = sequential_legal.second / parallel_legal.second;
            
            std::cout << "  Pseudo-legal moves: " << sequential_pseudo.first << std::endl;
            std::cout << "    Sequential: " << std::fixed << std::setprecision(2) << sequential_pseudo.second << " μs" << std::endl;
            std::cout << "    Parallel:   " << std::fixed << std::setprecision(2) << parallel_pseudo.second << " μs" << std::endl;
            std::cout << "    Speedup:    " << std::fixed << std::setprecision(2) << pseudo_speedup << "x" << std::endl;
            
            std::cout << "  Legal moves: " << sequential_legal.first << std::endl;
            std::cout << "    Sequential: " << std::fixed << std::setprecision(2) << sequential_legal.second << " μs" << std::endl;
            std::cout << "    Parallel:   " << std::fixed << std::setprecision(2) << parallel_legal.second << " μs" << std::endl;
            std::cout << "    Speedup:    " << std::fixed << std::setprecision(2) << legal_speedup << "x" << std::endl;
            
            std::cout << std::endl;
        }
        
        // Print summary
        print_benchmark_summary(results);
    }
    
private:
    static std::pair<int, double> benchmark_pseudo_legal_sequential(const Position& pos) {
        const int iterations = 1000;
        S_MOVELIST moves;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            generate_all_moves(pos, moves);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        return {moves.count, duration.count() / (1000.0 * iterations)};  // Return count and average microseconds
    }
    
    static std::pair<int, double> benchmark_pseudo_legal_parallel(const Position& pos) {
        const int iterations = 1000;
        S_MOVELIST moves;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            ParallelMoveGenerator::generate_all_moves_parallel(pos, moves);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        return {moves.count, duration.count() / (1000.0 * iterations)};
    }
    
    static std::pair<int, double> benchmark_legal_sequential(Position& pos) {
        const int iterations = 100;  // Fewer iterations since legal move generation is slower
        S_MOVELIST moves;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            generate_legal_moves_enhanced(pos, moves);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        return {moves.count, duration.count() / (1000.0 * iterations)};
    }
    
    static std::pair<int, double> benchmark_legal_parallel(Position& pos) {
        const int iterations = 100;
        S_MOVELIST moves;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            ParallelMoveGenerator::generate_legal_moves_parallel(pos, moves);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        return {moves.count, duration.count() / (1000.0 * iterations)};
    }
    
    static void print_benchmark_summary(const std::vector<BenchmarkResult>& results) {
        std::cout << "=== BENCHMARK SUMMARY ===" << std::endl;
        std::cout << std::left << std::setw(20) << "Position" 
                  << std::setw(18) << "Method" 
                  << std::setw(8) << "Moves"
                  << std::setw(12) << "Time (μs)"
                  << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        
        for (const auto& result : results) {
            std::cout << std::left << std::setw(20) << result.position_name
                      << std::setw(18) << result.method
                      << std::setw(8) << result.move_count
                      << std::fixed << std::setprecision(2) << std::setw(12) << result.time_microseconds
                      << std::endl;
        }
    }
};

int main() {
    std::cout << "Huginn Chess Engine - Move Generation Parallelization Benchmark" << std::endl;
    std::cout << "=================================================================" << std::endl;
    
    try {
        MoveGenBenchmark::run_comprehensive_benchmark();
        
        std::cout << std::endl << "=== ANALYSIS AND RECOMMENDATIONS ===" << std::endl;
        std::cout << "1. Parallel move generation is most beneficial for complex positions" << std::endl;
        std::cout << "2. Legal move checking shows better parallelization potential" << std::endl;
        std::cout << "3. Consider position complexity when deciding to use parallel methods" << std::endl;
        std::cout << "4. For UCI search, focus on parallel search rather than parallel move generation" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Benchmark failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
