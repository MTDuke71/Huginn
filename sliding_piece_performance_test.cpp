#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "position.hpp"
#include "movegen_enhanced.hpp"

int main() {
    std::vector<std::string> test_positions = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",  // Starting position
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",  // Kiwipete
        "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 0 1",  // Open position
        "r1bq1rk1/pp2ppbp/2np1np1/8/3PP3/2N1BN2/PPP1BPPP/R2QK2R w KQ - 0 1",  // Tactical position
        "8/8/8/8/8/2k5/8/2K1Q3 w - - 0 1",  // Queen endgame
        "8/8/8/8/8/2k5/8/2KR4 w - - 0 1",  // Rook endgame
        "8/8/8/3n4/8/2k5/8/2KBB3 w - - 0 1",  // Bishop pair
        "8/2k5/8/8/8/8/2K5/8 w - - 0 1"  // No sliding pieces
    };
    
    const int iterations = 10000;
    int total_moves = 0;
    
    std::cout << "=== Sliding Piece Optimization Performance Test ===" << std::endl;
    std::cout << "Testing " << iterations << " iterations across " << test_positions.size() << " positions" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < iterations; ++iter) {
        for (const auto& fen : test_positions) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST list;
            generate_all_moves(pos, list);
            total_moves += list.count;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "\nResults:" << std::endl;
    std::cout << "Total time: " << duration.count() << " μs" << std::endl;
    std::cout << "Total moves generated: " << total_moves << std::endl;
    std::cout << "Average time per position: " << std::fixed << std::setprecision(2) 
              << double(duration.count()) / (iterations * test_positions.size()) << " μs" << std::endl;
    std::cout << "Moves per second: " << std::fixed << std::setprecision(0) 
              << double(total_moves) / (double(duration.count()) / 1000000.0) << std::endl;
    
    // Test specific performance on positions with many sliding pieces
    std::cout << "\n=== High Sliding Piece Activity Positions ===" << std::endl;
    std::vector<std::string> sliding_heavy = {
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",  // Kiwipete
        "r1bq1rk1/pp2ppbp/2np1np1/8/3PP3/2N1BN2/PPP1BPPP/R2QK2R w KQ - 0 1"   // Tactical
    };
    
    const int heavy_iterations = 25000;
    int heavy_moves = 0;
    
    auto heavy_start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < heavy_iterations; ++iter) {
        for (const auto& fen : sliding_heavy) {
            Position pos;
            pos.set_from_fen(fen);
            S_MOVELIST list;
            generate_all_moves(pos, list);
            heavy_moves += list.count;
        }
    }
    
    auto heavy_end = std::chrono::high_resolution_clock::now();
    auto heavy_duration = std::chrono::duration_cast<std::chrono::microseconds>(heavy_end - heavy_start);
    
    std::cout << "Heavy sliding piece positions (" << heavy_iterations << " iterations):" << std::endl;
    std::cout << "Time: " << heavy_duration.count() << " μs" << std::endl;
    std::cout << "Average per position: " << std::fixed << std::setprecision(2)
              << double(heavy_duration.count()) / (heavy_iterations * sliding_heavy.size()) << " μs" << std::endl;
    
    return 0;
}
