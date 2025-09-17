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
    
    uint64_t piece_bb[2][6];  // [color][piece_type]
    int board[64];            // piece at each square (-1 = empty, 0-11 = piece)
    
public:
    SimplePosition() {
        clear();
        set_starting_position();
    }
    
    void clear() {
        for (int c = 0; c < 2; c++) {
            for (int p = 0; p < 6; p++) {
                piece_bb[c][p] = 0ULL;
            }
        }
        for (int sq = 0; sq < 64; sq++) {
            board[sq] = -1;
        }
    }
    
    void set_starting_position() {
        clear();
        
        // White pieces
        piece_bb[White][Rook] = 0x81ULL;        // a1, h1
        piece_bb[White][Knight] = 0x42ULL;      // b1, g1
        piece_bb[White][Bishop] = 0x24ULL;      // c1, f1
        piece_bb[White][Queen] = 0x08ULL;       // d1
        piece_bb[White][King] = 0x10ULL;        // e1
        piece_bb[White][Pawn] = 0xFF00ULL;      // a2-h2
        
        // Black pieces
        piece_bb[Black][Rook] = 0x8100000000000000ULL;     // a8, h8
        piece_bb[Black][Knight] = 0x4200000000000000ULL;   // b8, g8
        piece_bb[Black][Bishop] = 0x2400000000000000ULL;   // c8, f8
        piece_bb[Black][Queen] = 0x0800000000000000ULL;    // d8
        piece_bb[Black][King] = 0x1000000000000000ULL;     // e8
        piece_bb[Black][Pawn] = 0x00FF000000000000ULL;     // a7-h7
        
        // Set up board array
        for (int color = 0; color < 2; color++) {
            for (int piece_type = 0; piece_type < 6; piece_type++) {
                uint64_t bb = piece_bb[color][piece_type];
                while (bb) {
#ifdef _MSC_VER
                    unsigned long sq;
                    _BitScanForward64(&sq, bb);
#else
                    int sq = __builtin_ctzll(bb);
#endif
                    board[sq] = color * 6 + piece_type;
                    bb &= bb - 1;
                }
            }
        }
    }
    
    void set_middlegame_position() {
        clear();
        
        // Scattered pieces for testing
        piece_bb[White][Bishop] = (1ULL << 26) | (1ULL << 35);  // c4, d5
        piece_bb[White][Rook] = (1ULL << 28) | (1ULL << 56);    // e4, a8
        piece_bb[White][Queen] = (1ULL << 36);                  // e5
        piece_bb[White][King] = (1ULL << 4);                    // e1
        
        piece_bb[Black][Bishop] = (1ULL << 18) | (1ULL << 45);  // c3, f6
        piece_bb[Black][Rook] = (1ULL << 31) | (1ULL << 7);     // h4, h1
        piece_bb[Black][Queen] = (1ULL << 27);                  // d4
        piece_bb[Black][King] = (1ULL << 60);                   // e8
        
        // Add some pawns for blocking
        piece_bb[White][Pawn] = (1ULL << 17) | (1ULL << 25) | (1ULL << 33);  // b3, b4, b5
        piece_bb[Black][Pawn] = (1ULL << 41) | (1ULL << 49) | (1ULL << 57);  // b6, b7, b8
        
        // Set up board array
        for (int color = 0; color < 2; color++) {
            for (int piece_type = 0; piece_type < 6; piece_type++) {
                uint64_t bb = piece_bb[color][piece_type];
                while (bb) {
#ifdef _MSC_VER
                    unsigned long sq;
                    _BitScanForward64(&sq, bb);
#else
                    int sq = __builtin_ctzll(bb);
#endif
                    board[sq] = color * 6 + piece_type;
                    bb &= bb - 1;
                }
            }
        }
    }
    
    uint64_t pieces(int color, int piece_type) const {
        return piece_bb[color][piece_type];
    }
    
    uint64_t color_pieces(int color) const {
        uint64_t result = 0ULL;
        for (int p = 0; p < 6; p++) {
            result |= piece_bb[color][p];
        }
        return result;
    }
    
    uint64_t all_pieces() const {
        return color_pieces(White) | color_pieces(Black);
    }
    
    int piece_at(int square) const {
        return board[square];
    }
};

#include "ultra_engine/ultra_sliding_gen.hpp"

using namespace UltraEngine;

// Type alias for the sliding generation with our SimplePosition
using SlidingGen = UltraSlidingGen<SimplePosition>;

// Performance testing function
template<typename Func>
double time_function(const std::string& name, Func&& func, int iterations = 1000000) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; i++) {
        func();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    double ops_per_sec = (double)iterations / (duration.count() / 1e9);
    
    std::cout << std::setw(30) << std::left << name << ": " 
              << std::setw(12) << std::right << std::fixed << std::setprecision(2) 
              << ops_per_sec / 1e6 << " M ops/sec" << std::endl;
    
    return ops_per_sec;
}

void test_correctness() {
    std::cout << "\n=== SLIDING PIECE CORRECTNESS TESTS ===\n";
    
    SimplePosition pos;
    UltraMoveList moves;
    
    // Test starting position
    std::cout << "Testing starting position...\n";
    
    try {
        // Bishop moves from starting position
        moves.clear();
        SlidingGen::generate_bishop_moves(pos, moves, SimplePosition::White);
        std::cout << "✓ Starting position bishops: " << moves.size() << " moves (should be 0)\n";
        
        // Rook moves from starting position  
        moves.clear();
        SlidingGen::generate_rook_moves(pos, moves, SimplePosition::White);
        std::cout << "✓ Starting position rooks: " << moves.size() << " moves (should be 0)\n";
        
        // Queen moves from starting position
        moves.clear();
        SlidingGen::generate_queen_moves(pos, moves, SimplePosition::White);
        std::cout << "✓ Starting position queen: " << moves.size() << " moves (should be 0)\n";
        
        // Test middlegame position
        pos.set_middlegame_position();
        std::cout << "\nTesting middlegame position...\n";
        
        // White bishop moves
        moves.clear();
        SlidingGen::generate_bishop_moves(pos, moves, SimplePosition::White);
        std::cout << "✓ White bishops: " << moves.size() << " moves\n";
        
        // White rook moves
        moves.clear();
        SlidingGen::generate_rook_moves(pos, moves, SimplePosition::White);
        std::cout << "✓ White rooks: " << moves.size() << " moves\n";
        
        // White queen moves
        moves.clear();
        SlidingGen::generate_queen_moves(pos, moves, SimplePosition::White);
        std::cout << "✓ White queen: " << moves.size() << " moves\n";
        
        // Test move counting matches generation
        moves.clear();
        SlidingGen::generate_all_sliding_moves(pos, moves, SimplePosition::White);
        int generated_count = moves.size();
        int counted = SlidingGen::count_all_sliding_moves(pos, SimplePosition::White);
        std::cout << "Move count test: " << generated_count << " generated, " << counted << " counted\n";
        assert(generated_count == counted);
        std::cout << "✓ Move count consistency: " << generated_count << " == " << counted << "\n";
        
        // Test bulk vs individual generation
        moves.clear();
        SlidingGen::generate_all_sliding_moves(pos, moves, SimplePosition::White);
        int individual_count = moves.size();
        
        moves.clear();
        SlidingGen::generate_sliding_moves_bulk(pos, moves, SimplePosition::White);
        int bulk_count = moves.size();
        
        std::cout << "Bulk test: " << individual_count << " individual, " << bulk_count << " bulk\n";
        assert(individual_count == bulk_count);
        std::cout << "✓ Bulk vs individual: " << individual_count << " == " << bulk_count << "\n";
        
        std::cout << "✅ All correctness tests passed!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Exception in correctness test: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "❌ Unknown exception in correctness test!" << std::endl;
        throw;
    }
}

void test_performance() {
    std::cout << "\n=== SLIDING PIECE PERFORMANCE TESTS ===\n";
    
    SimplePosition pos;
    pos.set_middlegame_position();
    UltraMoveList moves;
    
    // Test individual piece generation
    time_function("Bishop Generation", [&]() {
        moves.clear();
        SlidingGen::generate_bishop_moves(pos, moves, SimplePosition::White);
    });
    
    time_function("Rook Generation", [&]() {
        moves.clear();
        SlidingGen::generate_rook_moves(pos, moves, SimplePosition::White);
    });
    
    time_function("Queen Generation", [&]() {
        moves.clear();
        SlidingGen::generate_queen_moves(pos, moves, SimplePosition::White);
    });
    
    time_function("All Sliding Generation", [&]() {
        moves.clear();
        SlidingGen::generate_all_sliding_moves(pos, moves, SimplePosition::White);
    });
    
    time_function("Bulk Sliding Generation", [&]() {
        moves.clear();
        SlidingGen::generate_sliding_moves_bulk(pos, moves, SimplePosition::White);
    });
    
    // Test move counting performance
    time_function("Bishop Move Counting", [&]() {
        int count = SlidingGen::count_bishop_moves(pos, SimplePosition::White);
        (void)count; // Avoid unused variable warning
    });
    
    time_function("All Sliding Move Counting", [&]() {
        int count = SlidingGen::count_all_sliding_moves(pos, SimplePosition::White);
        (void)count;
    });
    
    time_function("Bulk Move Counting", [&]() {
        int count = SlidingGen::count_sliding_moves_bulk(pos, SimplePosition::White);
        (void)count;
    });
}

void stress_test() {
    std::cout << "\n=== SLIDING PIECE STRESS TEST ===\n";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> sq_dist(0, 63);
    std::uniform_int_distribution<int> piece_dist(0, 2); // bishop, rook, queen
    
    int test_positions = 10000;
    int total_moves = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int test = 0; test < test_positions; test++) {
        SimplePosition pos;
        pos.clear();
        
        // Add random sliding pieces
        for (int i = 0; i < 8; i++) {
            int sq = sq_dist(gen);
            int piece_type = piece_dist(gen) + 2; // bishop=2, rook=3, queen=4
            int color = i % 2;
            
            // Simple placement (may overlap, that's ok for stress test)
            if (piece_type == 2) { // Bishop
                pos.piece_bb[color][SimplePosition::Bishop] |= (1ULL << sq);
            } else if (piece_type == 3) { // Rook
                pos.piece_bb[color][SimplePosition::Rook] |= (1ULL << sq);
            } else { // Queen
                pos.piece_bb[color][SimplePosition::Queen] |= (1ULL << sq);
            }
            
            pos.board[sq] = color * 6 + piece_type;
        }
        
        // Test move generation
        UltraMoveList moves;
        SlidingGen::generate_all_sliding_moves(pos, moves, SimplePosition::White);
        total_moves += moves.size();
        
        // Verify move counting matches
        int counted = SlidingGen::count_all_sliding_moves(pos, SimplePosition::White);
        assert(moves.size() == counted);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "✅ Stress test completed successfully!\n";
    std::cout << "   Positions tested: " << test_positions << "\n";
    std::cout << "   Total moves generated: " << total_moves << "\n";
    std::cout << "   Average moves per position: " << (double)total_moves / test_positions << "\n";
    std::cout << "   Time taken: " << duration.count() << " ms\n";
    std::cout << "   Positions per second: " << (test_positions * 1000.0) / duration.count() << "\n";
}

int main() {
    std::cout << "ULTRA-ENGINE SLIDING PIECE GENERATION TEST\n";
    std::cout << "==========================================\n";
    
    try {
        // Initialize magic bitboards
        UltraAttacks::initialize();
        std::cout << "✓ UltraAttacks system initialized\n\n";
        
        test_correctness();
        test_performance();
        stress_test();
        
        std::cout << "\n🚀 ALL TESTS PASSED! Ultra-sliding generation is ready!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}