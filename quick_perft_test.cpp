/**
 * @file quick_perft_test.cpp
 * @brief Quick performance test for UltraEngine with correct Kiwipete
 */
#include <iostream>
#include <chrono>
#include "../ultra_engine/ultra_position.hpp"
#include "../ultra_engine/ultra_move.hpp"
#include "../ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

uint64_t perft(UltraPosition& pos, int depth) {
    if (depth == 0) return 1;
    
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    
    uint64_t nodes = 0;
    for (int i = 0; i < moves.size(); i++) {
        const UltraMove& move = moves[i];
        if (pos.is_legal_move(move)) {
            // Make move (we'd need to implement this)
            // For now, just count legal moves at depth 1
            if (depth == 1) {
                nodes++;
            }
        }
    }
    
    return nodes;
}

int main() {
    UltraAttacks::initialize();
    
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    UltraPosition pos;
    if (!pos.set_fen(fen)) {
        std::cout << "Failed to parse FEN" << std::endl;
        return 1;
    }
    
    std::cout << "=== UltraEngine Quick Performance Test ===" << std::endl;
    std::cout << "Position: " << fen << std::endl;
    std::cout << std::endl;
    
    // Time the move generation
    auto start = std::chrono::high_resolution_clock::now();
    
    uint64_t total_moves = 0;
    const int iterations = 100000;
    
    for (int i = 0; i < iterations; i++) {
        UltraMoveList moves;
        pos.generate_all_moves(moves);
        
        for (int j = 0; j < moves.size(); j++) {
            if (pos.is_legal_move(moves[j])) {
                total_moves++;
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    uint64_t moves_per_iteration = total_moves / iterations;
    double time_per_iteration = duration.count() / (double)iterations;
    
    std::cout << "Results:" << std::endl;
    std::cout << "  Legal moves per position: " << moves_per_iteration << std::endl;
    std::cout << "  Iterations: " << iterations << std::endl;
    std::cout << "  Total time: " << duration.count() << " microseconds" << std::endl;
    std::cout << "  Time per move generation: " << time_per_iteration << " microseconds" << std::endl;
    std::cout << "  Move generations per second: " << (1000000.0 / time_per_iteration) << std::endl;
    
    return 0;
}