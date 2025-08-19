#include "board120.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

// Old calculation-based functions for comparison
constexpr inline File old_file_of(int sq120) {
    const int f = sq120 % 10;
    const int r = sq120 / 10;
    if (f < 1 || f > 8 || r < 2 || r > 9) return File::None;
    return File(f - 1);
}

constexpr inline Rank old_rank_of(int sq120) {
    const int f = sq120 % 10;
    const int r = sq120 / 10;
    if (f < 1 || f > 8 || r < 2 || r > 9) return Rank::None;
    return Rank(r - 2);
}

int main() {
    std::cout << "Performance comparison: Lookup tables vs calculations\n\n";
    
    // Test correctness first
    std::cout << "Correctness verification:\n";
    bool all_correct = true;
    for (int sq = 0; sq < 120; ++sq) {
        File new_f = file_of(sq);
        File old_f = old_file_of(sq);
        Rank new_r = rank_of(sq);
        Rank old_r = old_rank_of(sq);
        
        if (new_f != old_f || new_r != old_r) {
            std::cout << "Mismatch at square " << sq << "!\n";
            all_correct = false;
        }
    }
    std::cout << (all_correct ? "✓ All results match!\n\n" : "✗ Results differ!\n\n");
    
    // Performance test
    const int iterations = 10000000;
    volatile File dummy_f = File::A;  // volatile to prevent optimization
    volatile Rank dummy_r = Rank::R1;
    
    // Test lookup tables
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        for (int sq = 21; sq <= 98; ++sq) {  // All playable squares
            dummy_f = file_of(sq);
            dummy_r = rank_of(sq);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto lookup_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Test calculations
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        for (int sq = 21; sq <= 98; ++sq) {  // All playable squares
            dummy_f = old_file_of(sq);
            dummy_r = old_rank_of(sq);
        }
    }
    end = std::chrono::high_resolution_clock::now();
    auto calc_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Performance results (" << iterations << " iterations, 64 squares each):\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Lookup tables: " << lookup_time.count() << " μs\n";
    std::cout << "Calculations:  " << calc_time.count() << " μs\n";
    
    if (calc_time.count() > 0) {
        double speedup = static_cast<double>(calc_time.count()) / lookup_time.count();
        std::cout << "Speedup: " << speedup << "x faster\n";
    }
    
    // Suppress unused variable warning
    (void)dummy_f;
    (void)dummy_r;
    
    return 0;
}
