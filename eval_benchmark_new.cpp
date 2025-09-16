#include "minimal_search.hpp"
#include "position.hpp"
#include "init.hpp"
#include "evaluation.hpp"
#include <chrono>
#include <iostream>

int main() {
    // Initialize the chess engine
    Huginn::init();
    
    // Create a position object
    Position pos;
    
    // Set up a complex middle game position 
    std::string fen = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NP1/PPP1NPB1/R1BQ1RK1 b - - 0 9";
    pos.set_from_fen(fen);
    
    // Create MinimalEngine instance
    Huginn::MinimalEngine engine;
    
    // Warm up
    for (int i = 0; i < 1000; ++i) {
        volatile int score = engine.evaluate(pos);
        (void)score; // Suppress unused warning
    }
    
    // Benchmark: Run evaluation many times and measure time
    const int NUM_ITERATIONS = 100000;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        volatile int score = engine.evaluate(pos);
        (void)score; // Suppress unused warning
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    double total_time_ms = duration.count() / 1000.0;
    double avg_time_per_eval = duration.count() / static_cast<double>(NUM_ITERATIONS);
    
    std::cout << "=== EVALUATION PERFORMANCE BENCHMARK ===" << std::endl;
    std::cout << "Position: " << fen << std::endl;
    std::cout << "Iterations: " << NUM_ITERATIONS << std::endl;
    std::cout << "Total time: " << total_time_ms << " ms" << std::endl;
    std::cout << "Average time per evaluation: " << avg_time_per_eval << " microseconds" << std::endl;
    std::cout << "Evaluations per second: " << (NUM_ITERATIONS * 1000000.0) / duration.count() << std::endl;
    
    return 0;
}