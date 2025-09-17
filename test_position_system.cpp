#include <iostream>
#include <chrono>
#include <cassert>
#include <iomanip>
#include <functional>
#include "ultra_engine/ultra_position.hpp"
#include "ultra_engine/ultra_attacks.hpp"

using namespace UltraEngine;

void test_position_setup() {
    std::cout << "\n=== POSITION SETUP TESTS ===\n";
    
    UltraPosition pos;
    
    // Test 1: Starting position
    std::cout << "Testing starting position setup...\n";
    pos.setup_starting_position();
    
    // Verify piece placement
    assert(pos.get_piece_type_at(0) == UltraPosition::Rook);   // a1
    assert(pos.get_piece_color_at(0) == UltraPosition::White);
    assert(pos.get_piece_type_at(4) == UltraPosition::King);   // e1
    assert(pos.get_piece_color_at(4) == UltraPosition::White);
    assert(pos.get_piece_type_at(60) == UltraPosition::King);  // e8
    assert(pos.get_piece_color_at(60) == UltraPosition::Black);
    
    std::cout << "✓ Starting position pieces placed correctly\n";
    
    // Test 2: FEN parsing
    std::cout << "Testing FEN parsing...\n";
    const std::string test_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    assert(pos.set_fen(test_fen));
    
    std::string generated_fen = pos.get_fen();
    std::cout << "Original FEN:  " << test_fen << "\n";
    std::cout << "Generated FEN: " << generated_fen << "\n";
    assert(generated_fen == test_fen);
    
    std::cout << "✓ FEN parsing and generation working correctly\n";
    
    // Test 3: Complex position
    std::cout << "Testing complex position...\n";
    const std::string complex_fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    assert(pos.set_fen(complex_fen));
    assert(pos.get_fen() == complex_fen);
    
    std::cout << "✓ Complex position handled correctly\n";
}

void test_move_generation() {
    std::cout << "\n=== MOVE GENERATION TESTS ===\n";
    
    UltraPosition pos;
    UltraMoveList moves;
    
    // Test 1: Starting position
    std::cout << "Testing starting position move generation...\n";
    pos.setup_starting_position();
    
    moves.clear();
    int total_moves = pos.generate_all_moves(moves);
    std::cout << "✓ Starting position total moves: " << total_moves << " (should be 20)\n";
    assert(total_moves == 20);
    
    moves.clear();
    int quiet_moves = pos.generate_quiet_moves(moves);
    std::cout << "✓ Starting position quiet moves: " << quiet_moves << " (should be 20)\n";
    assert(quiet_moves == 20);
    
    moves.clear();
    int captures = pos.generate_captures(moves);
    std::cout << "✓ Starting position captures: " << captures << " (should be 0)\n";
    assert(captures == 0);
    
    // Test 2: Move counting consistency
    int counted_moves = pos.count_all_moves();
    std::cout << "✓ Move count consistency: " << total_moves << " == " << counted_moves << "\n";
    assert(total_moves == counted_moves);
    
    // Test 3: Complex position with captures
    std::cout << "Testing position with captures...\n";
    pos.set_fen("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
    
    moves.clear();
    total_moves = pos.generate_all_moves(moves);
    counted_moves = pos.count_all_moves();
    std::cout << "✓ Complex position moves: " << total_moves << " generated, " << counted_moves << " counted\n";
    assert(total_moves == counted_moves);
    
    std::cout << "✅ All move generation tests passed!\n";
}

void test_make_unmake() {
    std::cout << "\n=== MAKE/UNMAKE MOVE TESTS ===\n";
    
    UltraPosition pos;
    pos.setup_starting_position();
    
    // Store original state
    std::string original_fen = pos.get_fen();
    std::cout << "Original position: " << original_fen << "\n";
    
    // Generate a move and make it
    UltraMoveList moves;
    pos.generate_all_moves(moves);
    assert(moves.size() > 0);
    
    const UltraMove test_move = moves[0]; // e2-e4 typically
    std::cout << "Making move: " << test_move.from() << "-" << test_move.to() << "\n";
    
    pos.make_move(test_move);
    std::string after_move_fen = pos.get_fen();
    std::cout << "After move: " << after_move_fen << "\n";
    
    // Verify the position changed
    assert(after_move_fen != original_fen);
    
    // Unmake the move
    pos.unmake_move(test_move);
    std::string restored_fen = pos.get_fen();
    std::cout << "After unmake: " << restored_fen << "\n";
    
    // Verify position is restored
    assert(restored_fen == original_fen);
    std::cout << "✓ Make/unmake preserves position correctly\n";
    
    // Test multiple moves
    std::cout << "Testing sequence of moves...\n";
    pos.generate_all_moves(moves);
    
    std::vector<UltraMove> move_sequence;
    for (int i = 0; i < std::min(5, moves.size()); i++) {
        move_sequence.push_back(moves[i]);
        pos.make_move(moves[i]);
    }
    
    // Unmake all moves in reverse order
    for (int i = move_sequence.size() - 1; i >= 0; i--) {
        pos.unmake_move(move_sequence[i]);
    }
    
    assert(pos.get_fen() == original_fen);
    std::cout << "✓ Multiple make/unmake sequence works correctly\n";
    
    std::cout << "✅ All make/unmake tests passed!\n";
}

void test_performance() {
    std::cout << "\n=== ULTRA-POSITION PERFORMANCE TESTS ===\n";
    
    UltraPosition pos;
    pos.setup_starting_position();
    
    const int ITERATIONS = 1000000;
    UltraMoveList moves;
    
    auto time_operation = [&](const std::string& name, auto operation) {
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < ITERATIONS; i++) {
            operation();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        double ops_per_sec = static_cast<double>(ITERATIONS) / (duration / 1e9);
        
        std::cout << std::left << std::setw(30) << name + ":" 
                  << std::right << std::setw(12) << std::fixed << std::setprecision(2) 
                  << (ops_per_sec / 1e6) << " M ops/sec\n";
    };
    
    // Test different position operations
    time_operation("Complete Move Generation", [&]() {
        moves.clear();
        pos.generate_all_moves(moves);
    });
    
    time_operation("Move Counting Only", [&]() {
        pos.count_all_moves();
    });
    
    time_operation("Capture Generation", [&]() {
        moves.clear();
        pos.generate_captures(moves);
    });
    
    time_operation("Quiet Move Generation", [&]() {
        moves.clear();
        pos.generate_quiet_moves(moves);
    });
    
    // Test make/unmake performance
    UltraMove test_move = UltraMove::quiet(12, 28); // e2-e4
    
    time_operation("Make Move", [&]() {
        pos.make_move(test_move);
        pos.unmake_move(test_move);
    });
    
    time_operation("Position Copy", [&]() {
        UltraPosition temp = pos;
        (void)temp; // Suppress unused variable warning
    });
    
    time_operation("FEN Generation", [&]() {
        std::string fen = pos.get_fen();
        (void)fen; // Suppress unused variable warning
    });
}

void test_perft_integration() {
    std::cout << "\n=== PERFT INTEGRATION TEST ===\n";
    
    UltraPosition pos;
    pos.setup_starting_position();
    
    // Simple perft implementation to test integration
    std::function<uint64_t(UltraPosition&, int)> perft = [&](UltraPosition& position, int depth) -> uint64_t {
        if (depth == 0) return 1;
        
        UltraMoveList moves;
        int move_count = position.generate_all_moves(moves);
        
        uint64_t nodes = 0;
        for (int i = 0; i < move_count; i++) {
            position.make_move(moves[i]);
            nodes += perft(position, depth - 1);
            position.unmake_move(moves[i]);
        }
        
        return nodes;
    };
    
    // Test known perft results
    auto start = std::chrono::high_resolution_clock::now();
    
    uint64_t perft1 = perft(pos, 1);
    uint64_t perft2 = perft(pos, 2);
    uint64_t perft3 = perft(pos, 3);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "Perft(1): " << perft1 << " (should be 20)\n";
    std::cout << "Perft(2): " << perft2 << " (should be 400)\n";
    std::cout << "Perft(3): " << perft3 << " (should be 8902)\n";
    std::cout << "Time taken: " << duration << " ms\n";
    
    assert(perft1 == 20);
    assert(perft2 == 400);
    assert(perft3 == 8902);
    
    std::cout << "✅ Perft integration test passed!\n";
}

int main() {
    std::cout << "ULTRA-ENGINE POSITION SYSTEM TEST\n";
    std::cout << "=================================\n";
    
    try {
        // Initialize attack tables
        UltraAttacks::initialize();
        std::cout << "✓ UltraAttacks system initialized\n";
        
        test_position_setup();
        test_move_generation();
        test_make_unmake();
        test_performance();
        test_perft_integration();
        
        std::cout << "\n🚀 ALL TESTS PASSED! Ultra-position system is ready!\n";
        std::cout << "🎯 Ready for complete ultra-engine perft implementation!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}