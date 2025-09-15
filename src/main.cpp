#include "uci.hpp"
#ifdef BITBOARD_ENGINE
#include "bitboard_benchmark.hpp"
#include "position.hpp"
#include <iostream>
#include <string>
#endif

/**
 * @brief Entry point of the application.
 *
 * Initializes the UCI (Universal Chess Interface) and starts its main loop.
 * For huginn2, also supports bitboard benchmarking commands.
 * 
 * @return int Returns 0 upon successful execution.
 */

int main(int argc, char* argv[]) {
#ifdef BITBOARD_ENGINE
    // Check for bitboard benchmark command
    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        std::cout << "=== Huginn2 Bitboard Benchmark Mode ===" << std::endl;
        
        // Create a test position
        Position pos;
        pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        
        // Run comprehensive benchmark
        BitboardBenchmark::run_comprehensive_benchmark(pos);
        return 0;
    }
#endif

    UCIInterface uci;
    uci.run();
    return 0;
}
