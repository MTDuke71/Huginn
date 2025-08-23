#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <iomanip>
#include "move.hpp"
#include "chess_types.hpp"

// Performance test comparing decode_move vs getter methods
class MoveDecodingBenchmark {
private:
    std::vector<S_MOVE> test_moves;
    static constexpr int NUM_MOVES = 1000000;
    static constexpr int NUM_ITERATIONS = 100;

public:
    void generate_test_moves() {
        test_moves.clear();
        test_moves.reserve(NUM_MOVES);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> square_dist(21, 98);  // Valid mailbox squares
        std::uniform_int_distribution<> piece_dist(0, 5);     // PieceType values
        std::uniform_int_distribution<> bool_dist(0, 1);      // Boolean flags
        
        for (int i = 0; i < NUM_MOVES; ++i) {
            int from = square_dist(gen);
            int to = square_dist(gen);
            PieceType captured = PieceType(piece_dist(gen));
            bool en_passant = bool_dist(gen);
            bool pawn_start = bool_dist(gen);
            PieceType promoted = PieceType(piece_dist(gen));
            bool castle = bool_dist(gen);
            
            test_moves.emplace_back(from, to, captured, en_passant, pawn_start, promoted, castle);
        }
        
        std::cout << "Generated " << NUM_MOVES << " test moves\n";
    }
    
    // Test using decode_move function
    double test_decode_move() {
        auto start = std::chrono::high_resolution_clock::now();
        
        volatile int total_from = 0, total_to = 0;
        volatile int total_promo = 0;
        
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {
            for (const auto& move : test_moves) {
                int from, to;
                PieceType promo;
                S_MOVE::decode_move(move.move, from, to, promo);
                
                // Use the results to prevent optimization
                total_from += from;
                total_to += to;
                total_promo += int(promo);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        // Prevent compiler from optimizing away the calculations
        if (total_from == 0 && total_to == 0 && total_promo == 0) {
            std::cout << "Impossible result\n";
        }
        
        return double(duration.count()) / (NUM_MOVES * NUM_ITERATIONS);
    }
    
    // Test using modern getter methods
    double test_getter_methods() {
        auto start = std::chrono::high_resolution_clock::now();
        
        volatile int total_from = 0, total_to = 0;
        volatile int total_promo = 0;
        
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {
            for (const auto& move : test_moves) {
                int from = move.get_from();
                int to = move.get_to();
                PieceType promo = move.get_promoted();
                
                // Use the results to prevent optimization
                total_from += from;
                total_to += to;
                total_promo += int(promo);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        // Prevent compiler from optimizing away the calculations
        if (total_from == 0 && total_to == 0 && total_promo == 0) {
            std::cout << "Impossible result\n";
        }
        
        return double(duration.count()) / (NUM_MOVES * NUM_ITERATIONS);
    }
    
    // Test accessing all fields using getter methods
    double test_full_getter_methods() {
        auto start = std::chrono::high_resolution_clock::now();
        
        volatile int total_from = 0, total_to = 0, total_captured = 0, total_promo = 0;
        volatile int total_flags = 0;
        
        for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {
            for (const auto& move : test_moves) {
                int from = move.get_from();
                int to = move.get_to();
                PieceType captured = move.get_captured();
                PieceType promo = move.get_promoted();
                bool en_passant = move.is_en_passant();
                bool pawn_start = move.is_pawn_start();
                bool castle = move.is_castle();
                
                // Use the results to prevent optimization
                total_from += from;
                total_to += to;
                total_captured += int(captured);
                total_promo += int(promo);
                total_flags += (en_passant ? 1 : 0) + (pawn_start ? 1 : 0) + (castle ? 1 : 0);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        
        // Prevent compiler from optimizing away the calculations
        if (total_from == 0 && total_to == 0) {
            std::cout << "Impossible result\n";
        }
        
        return double(duration.count()) / (NUM_MOVES * NUM_ITERATIONS);
    }
    
    void run_benchmark() {
        std::cout << "=== Move Decoding Performance Benchmark ===\n";
        std::cout << "Testing " << NUM_MOVES << " moves x " << NUM_ITERATIONS << " iterations\n\n";
        
        generate_test_moves();
        
        // Warm up
        std::cout << "Warming up...\n";
        test_decode_move();
        test_getter_methods();
        
        std::cout << "\nRunning benchmarks...\n";
        
        // Test decode_move function
        double decode_time = test_decode_move();
        std::cout << "decode_move():           " << std::fixed << std::setprecision(2) 
                  << decode_time << " ns/call\n";
        
        // Test getter methods
        double getter_time = test_getter_methods();
        std::cout << "getter methods:          " << std::fixed << std::setprecision(2) 
                  << getter_time << " ns/call\n";
        
        // Test full getter methods
        double full_getter_time = test_full_getter_methods();
        std::cout << "full getter methods:     " << std::fixed << std::setprecision(2) 
                  << full_getter_time << " ns/call\n";
        
        // Analysis
        std::cout << "\n=== Analysis ===\n";
        double improvement = ((decode_time - getter_time) / decode_time) * 100.0;
        std::cout << "Performance improvement: " << std::fixed << std::setprecision(1) 
                  << improvement << "%\n";
        
        if (getter_time < decode_time) {
            std::cout << "✓ Getter methods are FASTER by " 
                      << std::fixed << std::setprecision(2) 
                      << (decode_time / getter_time) << "x\n";
        } else {
            std::cout << "✗ decode_move is faster by " 
                      << std::fixed << std::setprecision(2) 
                      << (getter_time / decode_time) << "x\n";
        }
        
        std::cout << "\nRecommendation: Use " 
                  << (getter_time < decode_time ? "GETTER METHODS" : "decode_move()") 
                  << " for best performance\n";
    }
};

int main() {
    MoveDecodingBenchmark benchmark;
    benchmark.run_benchmark();
    return 0;
}
