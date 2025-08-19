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

constexpr inline bool old_is_playable(int sq120) {
    const int f = sq120 % 10;   // 0..9
    const int r = sq120 / 10;   // 0..11
    return (f >= 1 && f <= 8) && (r >= 2 && r <= 9);
}

constexpr inline bool old_is_offboard(int sq120) { return !old_is_playable(sq120); }

int main() {
    std::cout << "Comprehensive performance comparison: Lookup tables vs calculations\n\n";
    
    // Test correctness first
    std::cout << "Correctness verification:\n";
    bool all_correct = true;
    for (int sq = 0; sq < 120; ++sq) {
        if (file_of(sq) != old_file_of(sq) || 
            rank_of(sq) != old_rank_of(sq) ||
            is_playable(sq) != old_is_playable(sq) ||
            is_offboard(sq) != old_is_offboard(sq)) {
            std::cout << "Mismatch at square " << sq << "!\n";
            all_correct = false;
        }
    }
    std::cout << (all_correct ? "✓ All results match!\n\n" : "✗ Results differ!\n\n");
    
    // Performance test parameters
    const int iterations = 5000000;
    std::cout << "Performance results (" << iterations << " iterations, all 120 squares):\n";
    std::cout << std::fixed << std::setprecision(2);
    
    // Test file_of performance
    {
        volatile File dummy_f = File::A;
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            for (int sq = 0; sq < 120; ++sq) {
                dummy_f = file_of(sq);
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto lookup_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            for (int sq = 0; sq < 120; ++sq) {
                dummy_f = old_file_of(sq);
            }
        }
        end = std::chrono::high_resolution_clock::now();
        auto calc_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "file_of():\n";
        std::cout << "  Lookup: " << lookup_time.count() << " μs\n";
        std::cout << "  Calc:   " << calc_time.count() << " μs\n";
        if (calc_time.count() > 0) {
            double speedup = static_cast<double>(calc_time.count()) / lookup_time.count();
            std::cout << "  Speedup: " << speedup << "x\n\n";
        }
        (void)dummy_f;
    }
    
    // Test rank_of performance
    {
        volatile Rank dummy_r = Rank::R1;
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            for (int sq = 0; sq < 120; ++sq) {
                dummy_r = rank_of(sq);
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto lookup_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            for (int sq = 0; sq < 120; ++sq) {
                dummy_r = old_rank_of(sq);
            }
        }
        end = std::chrono::high_resolution_clock::now();
        auto calc_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "rank_of():\n";
        std::cout << "  Lookup: " << lookup_time.count() << " μs\n";
        std::cout << "  Calc:   " << calc_time.count() << " μs\n";
        if (calc_time.count() > 0) {
            double speedup = static_cast<double>(calc_time.count()) / lookup_time.count();
            std::cout << "  Speedup: " << speedup << "x\n\n";
        }
        (void)dummy_r;
    }
    
    // Test is_playable performance
    {
        volatile bool dummy_p = false;
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            for (int sq = 0; sq < 120; ++sq) {
                dummy_p = is_playable(sq);
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto lookup_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            for (int sq = 0; sq < 120; ++sq) {
                dummy_p = old_is_playable(sq);
            }
        }
        end = std::chrono::high_resolution_clock::now();
        auto calc_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "is_playable():\n";
        std::cout << "  Lookup: " << lookup_time.count() << " μs\n";
        std::cout << "  Calc:   " << calc_time.count() << " μs\n";
        if (calc_time.count() > 0) {
            double speedup = static_cast<double>(calc_time.count()) / lookup_time.count();
            std::cout << "  Speedup: " << speedup << "x\n\n";
        }
        (void)dummy_p;
    }
    
    std::cout << "Memory usage:\n";
    std::cout << "Lookup tables: " << (sizeof(FILE_RANK_LOOKUPS)) << " bytes total\n";
    std::cout << "  - files[120]: " << sizeof(FILE_RANK_LOOKUPS.files) << " bytes\n";
    std::cout << "  - ranks[120]: " << sizeof(FILE_RANK_LOOKUPS.ranks) << " bytes\n";
    std::cout << "  - playable[120]: " << sizeof(FILE_RANK_LOOKUPS.playable) << " bytes\n";
    
    return 0;
}
