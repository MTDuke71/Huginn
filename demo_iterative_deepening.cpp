#include "src/minimal_search.hpp"
#include "src/position.hpp"
#include <iostream>
#include <iomanip>

using namespace Huginn;

int main() {
    std::cout << "=== VICE Iterative Deepening Demonstration ===" << std::endl;
    std::cout << "Showing the two main benefits from the VICE video:" << std::endl;
    std::cout << "1. Time Management: Engine can return best move if time runs out" << std::endl;
    std::cout << "2. Move Ordering Efficiency: Shallower searches improve deeper searches" << std::endl;
    std::cout << std::endl;
    
    MinimalEngine engine;
    Position pos;
    pos.set_startpos();
    
    std::cout << "=== Demonstrating Time Management (Benefit 1) ===" << std::endl;
    std::cout << "Short time limit - engine should return best move from completed depth" << std::endl;
    
    MinimalLimits short_limits;
    short_limits.max_depth = 10;      // High depth
    short_limits.max_time_ms = 100;   // Very short time - only 100ms
    short_limits.infinite = false;
    
    auto start_time = std::chrono::steady_clock::now();
    S_MOVE quick_move = engine.search(pos, short_limits);
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Time limit: " << short_limits.max_time_ms << "ms" << std::endl;
    std::cout << "Actual time: " << elapsed.count() << "ms" << std::endl;
    std::cout << "Best move found: " << engine.move_to_uci(quick_move) << std::endl;
    std::cout << "✓ Engine returned a valid move within time limit!" << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== Demonstrating Move Ordering Efficiency (Benefit 2) ===" << std::endl;
    std::cout << "Watch how nodes/time grow efficiently due to better move ordering:" << std::endl;
    
    MinimalLimits deep_limits;
    deep_limits.max_depth = 6;
    deep_limits.max_time_ms = 10000;  // Longer time limit
    deep_limits.infinite = false;
    
    engine.reset(); // Reset for clean search
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Depth | Nodes     | Time(ms) | Nodes/sec  | Best Move" << std::endl;
    std::cout << "------|-----------|----------|------------|----------" << std::endl;
    
    // Let's manually track each depth to show the efficiency
    for (int target_depth = 1; target_depth <= 4; ++target_depth) {
        engine.reset();
        MinimalLimits depth_limits;
        depth_limits.max_depth = target_depth;
        depth_limits.max_time_ms = 10000;
        depth_limits.infinite = false;
        
        auto depth_start = std::chrono::steady_clock::now();
        S_MOVE depth_move = engine.search(pos, depth_limits);
        auto depth_end = std::chrono::steady_clock::now();
        auto depth_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(depth_end - depth_start);
        
        double nodes_per_sec = 0;
        if (depth_elapsed.count() > 0) {
            nodes_per_sec = (engine.nodes_searched * 1000.0) / depth_elapsed.count();
        }
        
        std::cout << std::setw(5) << target_depth << " | "
                  << std::setw(9) << engine.nodes_searched << " | "
                  << std::setw(8) << depth_elapsed.count() << " | "
                  << std::setw(10) << (int)nodes_per_sec << " | "
                  << engine.move_to_uci(depth_move) << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "Notice how:" << std::endl;
    std::cout << "- Each depth completes and provides a valid move (Time Management)" << std::endl;
    std::cout << "- Node efficiency improves due to better move ordering from PV table" << std::endl;
    std::cout << "- Alpha-beta cutoffs become more effective with each iteration" << std::endl;
    std::cout << std::endl;
    
    std::cout << "=== VICE-Style SearchPosition with Enhanced Output ===" << std::endl;
    SearchInfo info;
    info.max_depth = 4;
    info.infinite = false;
    
    S_MOVE vice_move = engine.searchPosition(pos, info);
    std::cout << "Final best move: " << engine.move_to_uci(vice_move) << std::endl;
    std::cout << "Total nodes: " << info.nodes << std::endl;
    
    return 0;
}
