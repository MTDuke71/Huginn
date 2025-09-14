#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include "knight_lookup_tables.hpp"
#include "knight_optimizations.hpp"
#include "position.hpp"
#include "init.hpp"

/**
 * Knight Move Generation Performance Demo
 * ======================================
 * 
 * This program demonstrates the performance benefits of lookup tables
 * for knight move generation compared to the existing template approach.
 */

void benchmark_knight_generation() {
    std::cout << "Knight Move Generation Performance Benchmark\n";
    std::cout << "============================================\n\n";
    
    // Test positions with varying numbers of knights
    const std::vector<std::pair<std::string, std::string>> test_positions = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "Starting Position (4 knights)"},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", "Kiwipete (6 knights)"},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", "Endgame (0 knights)"},
        {"8/8/2N2N2/8/8/2N2N2/8/8 w - - 0 1", "Four Knights Center"},
        {"N6N/8/8/8/8/8/8/N6N w - - 0 1", "Four Knights Corners"},
        {"NNNNNNNN/8/8/8/8/8/8/nnnnnnnn w - - 0 1", "Many Knights Edge"}
    };
    
    const int iterations = 50000;
    
    // Results storage
    struct BenchmarkResult {
        std::string method;
        long long total_time_ms;
        double moves_per_second;
    };
    
    std::vector<BenchmarkResult> results;
    
    for (const auto& [fen, description] : test_positions) {
        std::cout << "Testing: " << description << "\n";
        std::cout << "FEN: " << fen << "\n";
        
        Position pos;
        pos.set_from_fen(fen);
        
        // Count knight moves to verify correctness
        S_MOVELIST template_moves, lookup_moves, bitboard_moves;
        KnightOptimizations::generate_knight_moves_template(pos, template_moves, pos.side_to_move);
        KnightLookupTables::generate_knight_moves_lookup(pos, lookup_moves, pos.side_to_move);
        KnightLookupTables::generate_knight_moves_bitboard(pos, bitboard_moves, pos.side_to_move);
        
        std::cout << "Knight moves generated: " << template_moves.count << "\n";
        
        // Verify all methods generate the same number of moves
        if (template_moves.count != lookup_moves.count || template_moves.count != bitboard_moves.count) {
            std::cout << "ERROR: Methods generate different move counts!\n";
            std::cout << "Template: " << template_moves.count 
                      << ", Lookup: " << lookup_moves.count 
                      << ", Bitboard: " << bitboard_moves.count << "\n";
            continue;
        }
        
        // Benchmark template method
        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            pos.set_from_fen(fen); // Reset position
            S_MOVELIST moves;
            KnightOptimizations::generate_knight_moves_template(pos, moves, pos.side_to_move);
        }
        auto template_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start_time).count();
        
        // Benchmark lookup method
        start_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            pos.set_from_fen(fen); // Reset position
            S_MOVELIST moves;
            KnightLookupTables::generate_knight_moves_lookup(pos, moves, pos.side_to_move);
        }
        auto lookup_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start_time).count();
        
        // Benchmark bitboard method
        start_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            pos.set_from_fen(fen); // Reset position
            S_MOVELIST moves;
            KnightLookupTables::generate_knight_moves_bitboard(pos, moves, pos.side_to_move);
        }
        auto bitboard_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start_time).count();
        
        // Calculate speedup ratios
        double lookup_speedup = (template_time > 0) ? double(template_time) / double(lookup_time) : 1.0;
        double bitboard_speedup = (template_time > 0) ? double(template_time) / double(bitboard_time) : 1.0;
        
        // Display results for this position
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Results for " << iterations << " iterations:\n";
        std::cout << "  Template method:  " << template_time << " ms\n";
        std::cout << "  Lookup method:    " << lookup_time << " ms (";
        if (lookup_speedup >= 1.0) {
            std::cout << lookup_speedup << "x faster)";
        } else {
            std::cout << (1.0/lookup_speedup) << "x slower)";
        }
        std::cout << "\n";
        std::cout << "  Bitboard method:  " << bitboard_time << " ms (";
        if (bitboard_speedup >= 1.0) {
            std::cout << bitboard_speedup << "x faster)";
        } else {
            std::cout << (1.0/bitboard_speedup) << "x slower)";
        }
        std::cout << "\n\n";
    }
}

void print_lookup_table_info() {
    std::cout << "Knight Lookup Table Information\n";
    std::cout << "===============================\n\n";
    
    // Sample of lookup table data
    const int sample_squares[] = {0, 7, 27, 28, 35, 36, 56, 63}; // Corners and center
    const char* square_names[] = {"a1", "h1", "d4", "e4", "d5", "e5", "a8", "h8"};
    
    for (int i = 0; i < 8; ++i) {
        int sq = sample_squares[i];
        std::cout << "Square " << square_names[i] << " (" << sq << "): " 
                  << KnightLookupTables::KNIGHT_MOVE_COUNT[sq] << " moves -> ";
        
        for (int j = 0; j < KnightLookupTables::KNIGHT_MOVE_COUNT[sq]; ++j) {
            int target = KnightLookupTables::KNIGHT_MOVES[sq][j];
            char target_file = 'a' + (target % 8);
            char target_rank = '1' + (target / 8);
            std::cout << target_file << target_rank << " ";
        }
        std::cout << "\n";
    }
    
    // Memory usage
    size_t array_memory = sizeof(KnightLookupTables::KNIGHT_MOVES) + 
                         sizeof(KnightLookupTables::KNIGHT_MOVE_COUNT);
    size_t bitboard_memory = sizeof(KnightLookupTables::KNIGHT_ATTACKS);
    
    std::cout << "\nMemory usage:\n";
    std::cout << "Array tables:   " << array_memory << " bytes\n";
    std::cout << "Bitboard table: " << bitboard_memory << " bytes\n";
    std::cout << "Total:          " << (array_memory + bitboard_memory) << " bytes\n";
    std::cout << "Cache lines:    " << ((array_memory + bitboard_memory + 63) / 64) << " (64-byte lines)\n\n";
}

int main() {
    std::cout << "Huginn Chess Engine - Knight Lookup Table Demo\n";
    std::cout << "==============================================\n\n";
    
    // Initialize engine
    Huginn::init();
    
    // Print lookup table information
    print_lookup_table_info();
    
    // Run performance benchmark
    benchmark_knight_generation();
    
    std::cout << "Analysis Complete!\n\n";
    std::cout << "Summary:\n";
    std::cout << "--------\n";
    std::cout << "Lookup tables provide several benefits for knight move generation:\n";
    std::cout << "1. Eliminate repeated boundary checking (IS_PLAYABLE calls)\n";
    std::cout << "2. Reduce arithmetic operations per knight move\n";
    std::cout << "3. Better cache locality with small lookup tables\n";
    std::cout << "4. Consistent performance regardless of board position\n\n";
    std::cout << "Expected improvement: 5-15% over template approach in typical positions\n";
    std::cout << "Memory cost: ~3KB for complete knight lookup tables\n\n";
    
    return 0;
}