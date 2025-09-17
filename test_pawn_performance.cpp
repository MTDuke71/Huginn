#include <iostream>
#include <chrono>
#include <random>
#include <cassert>
#include <iomanip>

// Simple position class for testing (must be defined before template instantiation)
class SimplePosition {
public:
    // Piece type constants
    static constexpr int Pawn = 0;
    static constexpr int Knight = 1;
    static constexpr int Bishop = 2;
    static constexpr int Rook = 3;
    static constexpr int Queen = 4;
    static constexpr int King = 5;
    
    // Color constants  
    static constexpr int White = 0;
    static constexpr int Black = 1;
    
private:
    uint64_t piece_boards_[2][6] = {{0}};  // [color][piece_type]
    uint64_t color_boards_[2] = {0};       // [color] 
    uint64_t all_occupied_ = 0;
    int en_passant_square_ = -1;
    int to_move_ = White;
    
public:
    // Piece board access
    uint64_t get_piece_board(int color, int piece_type) const { 
        return piece_boards_[color][piece_type]; 
    }
    
    uint64_t get_color_board(int color) const { 
        return color_boards_[color]; 
    }
    
    uint64_t get_all_occupied() const { 
        return all_occupied_; 
    }
    
    int get_en_passant_square() const { 
        return en_passant_square_; 
    }
    
    int get_to_move() const { 
        return to_move_; 
    }
    
    // Position setup
    void set_piece(int square, int color, int piece_type) {
        const uint64_t sq_bb = 1ULL << square;
        piece_boards_[color][piece_type] |= sq_bb;
        color_boards_[color] |= sq_bb;
        all_occupied_ |= sq_bb;
    }
    
    void clear_square(int square) {
        const uint64_t sq_bb = ~(1ULL << square);
        for (int c = 0; c < 2; c++) {
            color_boards_[c] &= sq_bb;
            for (int p = 0; p < 6; p++) {
                piece_boards_[c][p] &= sq_bb;
            }
        }
        all_occupied_ &= sq_bb;
    }
    
    void set_en_passant(int square) {
        en_passant_square_ = square;
    }
    
    void set_to_move(int color) {
        to_move_ = color;
    }
    
    void clear() {
        for (int c = 0; c < 2; c++) {
            color_boards_[c] = 0;
            for (int p = 0; p < 6; p++) {
                piece_boards_[c][p] = 0;
            }
        }
        all_occupied_ = 0;
        en_passant_square_ = -1;
        to_move_ = White;
    }
    
    // Setup standard starting position
    void setup_starting_position() {
        clear();
        
        // White pieces
        for (int sq = 8; sq < 16; sq++) {
            set_piece(sq, White, Pawn);
        }
        
        // Black pieces  
        for (int sq = 48; sq < 56; sq++) {
            set_piece(sq, Black, Pawn);
        }
    }
    
    // Setup test position with pawns in various situations
    void setup_pawn_test_position() {
        clear();
        
        // White pawns: various situations
        set_piece(12, White, Pawn);  // e2 - can double push
        set_piece(19, White, Pawn);  // d3 - normal pawn
        set_piece(35, White, Pawn);  // d5 - can capture on c6/e6
        set_piece(54, White, Pawn);  // g7 - promotion available
        
        // Black pawns
        set_piece(42, Black, Pawn);  // c6 - can be captured
        set_piece(44, Black, Pawn);  // e6 - can be captured  
        set_piece(51, Black, Pawn);  // d7 - normal pawn
        set_piece(8, Black, Pawn);   // a2 - promotion available
        
        // Set up en passant opportunity
        set_en_passant(43); // d6 - en passant target
    }
};

#include "ultra_engine/ultra_pawn_gen.hpp"
#include "ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

void test_correctness() {
    std::cout << "\n=== PAWN MOVE CORRECTNESS TESTS ===\n";
    
    SimplePosition pos;
    UltraMoveList moves;
    
    // Test 1: Starting position - no pawn moves available
    std::cout << "Testing starting position...\n";
    pos.setup_starting_position();
    
    moves.clear();
    int white_moves = UltraPawnGen<SimplePosition>::generate_all(pos, moves, SimplePosition::White);
    std::cout << "✓ Starting position white pawns: " << white_moves << " moves (should be 16)\n";
    assert(white_moves == 16); // 8 single pushes + 8 double pushes
    
    moves.clear();
    int black_moves = UltraPawnGen<SimplePosition>::generate_all(pos, moves, SimplePosition::Black);
    std::cout << "✓ Starting position black pawns: " << black_moves << " moves (should be 16)\n";
    assert(black_moves == 16);
    
    // Test 2: Complex pawn position
    std::cout << "Testing complex pawn position...\n";
    pos.setup_pawn_test_position();
    
    moves.clear();
    white_moves = UltraPawnGen<SimplePosition>::generate_all(pos, moves, SimplePosition::White);
    std::cout << "✓ White pawn moves: " << white_moves << " moves\n";
    
    moves.clear();
    black_moves = UltraPawnGen<SimplePosition>::generate_all(pos, moves, SimplePosition::Black);
    std::cout << "✓ Black pawn moves: " << black_moves << " moves\n";
    
    // Test 3: Move counting consistency
    int white_counted = UltraPawnGen<SimplePosition>::count_moves(pos, SimplePosition::White);
    int black_counted = UltraPawnGen<SimplePosition>::count_moves(pos, SimplePosition::Black);
    
    std::cout << "Move count test: " << white_moves << " generated, " << white_counted << " counted\n";
    std::cout << "✓ White count consistency: " << white_moves << " == " << white_counted << "\n";
    assert(white_moves == white_counted);
    
    std::cout << "✓ Black count consistency: " << black_moves << " == " << black_counted << "\n";
    assert(black_moves == black_counted);
    
    // Test 4: Separate quiet vs captures
    moves.clear();
    int white_quiet = UltraPawnGen<SimplePosition>::generate_quiet(pos, moves, SimplePosition::White);
    
    moves.clear();
    int white_captures = UltraPawnGen<SimplePosition>::generate_captures(pos, moves, SimplePosition::White);
    
    std::cout << "Quiet vs captures: " << white_quiet << " + " << white_captures << " = " << (white_quiet + white_captures) << "\n";
    std::cout << "✓ Quiet + captures: " << (white_quiet + white_captures) << " == " << white_moves << "\n";
    assert(white_quiet + white_captures == white_moves);
    
    std::cout << "✅ All pawn correctness tests passed!\n";
}

void test_performance() {
    std::cout << "\n=== PAWN MOVE PERFORMANCE TESTS ===\n";
    
    SimplePosition pos;
    pos.setup_pawn_test_position();
    
    const int ITERATIONS = 10000000;
    UltraMoveList moves;
    
    auto time_operation = [&](const std::string& name, auto operation) {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < ITERATIONS; i++) {
            moves.clear();
            operation();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        double ops_per_sec = static_cast<double>(ITERATIONS) / (duration / 1e9);
        
        std::cout << std::left << std::setw(30) << name + ":" 
                  << std::right << std::setw(12) << std::fixed << std::setprecision(2) 
                  << (ops_per_sec / 1e6) << " M ops/sec\n";
    };
    
    // Test different pawn operations
    time_operation("White Pawn Generation", [&]() {
        UltraPawnGen<SimplePosition>::generate_all(pos, moves, SimplePosition::White);
    });
    
    time_operation("Black Pawn Generation", [&]() {
        UltraPawnGen<SimplePosition>::generate_all(pos, moves, SimplePosition::Black);
    });
    
    time_operation("White Pawn Pushes Only", [&]() {
        UltraPawnGen<SimplePosition>::generate_pushes(pos, moves, SimplePosition::White);
    });
    
    time_operation("White Pawn Captures Only", [&]() {
        UltraPawnGen<SimplePosition>::generate_captures(pos, moves, SimplePosition::White);
    });
    
    time_operation("White Pawn Count Only", [&]() {
        UltraPawnGen<SimplePosition>::count_moves(pos, SimplePosition::White);
    });
    
    time_operation("All Pawn Generation", [&]() {
        UltraPawnGen<SimplePosition>::generate_all(pos, moves, SimplePosition::White);
        UltraPawnGen<SimplePosition>::generate_all(pos, moves, SimplePosition::Black);
    });
    
    time_operation("All Pawn Counting", [&]() {
        UltraPawnGen<SimplePosition>::count_moves(pos, SimplePosition::White);
        UltraPawnGen<SimplePosition>::count_moves(pos, SimplePosition::Black);
    });
}

void stress_test() {
    std::cout << "\n=== PAWN MOVE STRESS TEST ===\n";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> square_dist(0, 63);
    std::uniform_int_distribution<> color_dist(0, 1);
    
    const int NUM_POSITIONS = 10000;
    int total_moves = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_POSITIONS; i++) {
        SimplePosition pos;
        UltraMoveList moves;
        
        // Create random pawn position
        int num_pawns = square_dist(gen) % 16 + 2; // 2-17 pawns
        for (int j = 0; j < num_pawns; j++) {
            int square = square_dist(gen);
            int color = color_dist(gen);
            pos.set_piece(square, color, SimplePosition::Pawn);
        }
        
        // Generate moves for both colors
        int white_moves = UltraPawnGen<SimplePosition>::generate_all(pos, moves, SimplePosition::White);
        int black_moves = UltraPawnGen<SimplePosition>::generate_all(pos, moves, SimplePosition::Black);
        
        // Verify counting is consistent
        int white_counted = UltraPawnGen<SimplePosition>::count_moves(pos, SimplePosition::White);
        int black_counted = UltraPawnGen<SimplePosition>::count_moves(pos, SimplePosition::Black);
        
        assert(white_moves == white_counted);
        assert(black_moves == black_counted);
        
        total_moves += white_moves + black_moves;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "✅ Stress test completed successfully!\n";
    std::cout << "   Positions tested: " << NUM_POSITIONS << "\n";
    std::cout << "   Total moves generated: " << total_moves << "\n";
    std::cout << "   Average moves per position: " << std::fixed << std::setprecision(2) 
              << (double)total_moves / NUM_POSITIONS << "\n";
    std::cout << "   Time taken: " << duration << " ms\n";
    std::cout << "   Positions per second: " << std::fixed << std::setprecision(2) 
              << (double)NUM_POSITIONS / (duration / 1000.0) << "\n";
}

int main() {
    std::cout << "ULTRA-ENGINE PAWN MOVE GENERATION TEST\n";
    std::cout << "=====================================\n";
    
    try {
        // Initialize magic bitboards
        UltraAttacks::initialize();
        std::cout << "✓ UltraAttacks system initialized\n";
        
        test_correctness();
        test_performance();
        stress_test();
        
        std::cout << "\n🚀 ALL TESTS PASSED! Ultra-pawn generation is ready!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}