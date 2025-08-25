#include "movegen_profiler.hpp"
#include "init.hpp"
#include <iostream>
#include <cassert>

// Simple validation test for the profiler
void test_profiler_accuracy() {
    std::cout << "Testing profiler accuracy..." << std::endl;
    
    // Initialize engine
    Huginn::init();
    assert(Huginn::is_initialized());
    
    // Test with starting position
    Position pos;
    pos.set_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // Generate moves normally
    S_MOVELIST normal_moves;
    generate_all_moves(pos, normal_moves);
    
    // Generate moves with profiler
    S_MOVELIST profiled_moves;
    MoveGenProfiler::profile_generate_all_moves(pos, profiled_moves);
    
    // Verify move counts match
    assert(normal_moves.count == profiled_moves.count);
    
    // Verify moves are identical
    for (int i = 0; i < normal_moves.count; ++i) {
        assert(normal_moves[i].move == profiled_moves[i].move);
    }
    
    std::cout << "✓ Profiler accuracy test passed!" << std::endl;
    std::cout << "  Normal generation: " << normal_moves.count << " moves" << std::endl;
    std::cout << "  Profiled generation: " << profiled_moves.count << " moves" << std::endl;
}

int main() {
    std::cout << "Huginn Chess Engine - Profiler Validation Test" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    try {
        test_profiler_accuracy();
        std::cout << std::endl << "All tests passed! Profiler is accurate." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}